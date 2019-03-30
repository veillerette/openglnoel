#include "Scene.h"

#include <queue>

SceneDescriptor::SceneDescriptor(const char *path, Shader *sShader) : model(), vaos(), activeTextures(true), shadowShader(sShader) {
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

	if (!warn.empty()) {
		std::cout << "Warn: " << warn << std::endl;
	}

	if (!err.empty()) {
		std::cout << "Err: " << err << std::endl;
	}

	if (!ret) {
		std::cout << "Failed to parse glTF" << std::endl;
		exit(EXIT_FAILURE);
	}

	computeBox();
}

GLuint SceneDescriptor::loadTexture(tinygltf::Texture texture) {
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	tinygltf::Image img = model.images[texture.source];

	std::cout << "load image " << img.uri << " width=" << img.width << " height=" << img.height <<
			  " as_is=" << img.as_is << std::endl;

	GLenum format = GL_RGBA;

	switch (img.component) {
		case 1:
			format = GL_RED;
			break;
		case 2:
			format = GL_RG;
			break;
		case 3:
			format = GL_RGB;
			break;
	}
	std::cout << "image data : " << img.image.size() << std::endl;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, format, GL_UNSIGNED_BYTE, &img.image.at(0));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

void SceneDescriptor::loadAllTextures() {
	for (int i = 0; i < model.textures.size(); i++) {
		std::cout << "loading texture n°" << i << std::endl;
		textures[i] = loadTexture(model.textures[i]);
	}
}

static glm::mat4 double_to_mat4(double * v) {
    return glm::mat4(
            v[0], v[1], v[2], v[3],
            v[4], v[5], v[6], v[7],
            v[8], v[9], v[10], v[11],
            v[12], v[13], v[14], v[15]
            );
}

static glm::vec3 double_to_vec3(double * v) {
    return glm::vec3(v[0], v[1], v[2]);
}

static glm::vec4 double_to_vec4(double * v) {
    return glm::vec4(v[0], v[1], v[2], v[3]);
}

static glm::vec4 float_to_vec4(float * v) {
    return glm::vec4(v[0], v[1], v[2], v[3]);
}

void SceneDescriptor::loadPoses() {
    skin.poses.clear();
    for(int i = 0; i < model.nodes.size(); ++i) {
        auto node = model.nodes[i];

        if(node.mesh >= 0)
            rig[node.mesh] = i;
        skin.nb_meshes = std::max(skin.nb_meshes, node.mesh + 1);

        if(skin.root < 0)
            skin.root = i;
        for(int c : node.children) {
            if(c == skin.root)
                skin.root = i;
        }

        if(node.matrix.size() != 0) {
            skin.poses.push_back(NodePose(glm::mat4(double_to_mat4(node.matrix.data()))));
        } else {
            skin.poses.push_back(NodePose(
                    node.rotation.size() != 0    ? double_to_vec4(node.rotation.data())    : glm::vec4(0, 0, 0, 1),
                    node.scale.size() != 0       ? double_to_vec3(node.scale.data())       : glm::vec3(1, 1, 1),
                    node.translation.size() != 0 ? double_to_vec3(node.translation.data()) : glm::vec3(0, 0, 0)
                    ));
        }
    }
    skin.poses[skin.root].parent = -1;
    for(int i = 0; i < model.nodes.size(); ++i) {
        auto node = model.nodes[i];
        for(int c : node.children) {
            skin.poses[c].parent = i;
        }
    }
    computePosesTransforms();
    //skin.debug(model);
}

void SceneDescriptor::refreshPoses() {
    float time = skin.frame;
    skin.refresh();
    //std::cout << "refresh : " << time << std::endl;
    for(auto anim : model.animations) {
        for(auto channel : anim.channels) {
            NodePose & node = skin.poses[channel.target_node];
            auto & sampler = anim.samplers[channel.sampler];

            tinygltf::Accessor input = model.accessors[sampler.input];
            tinygltf::BufferView input_view = model.bufferViews[input.bufferView];
            tinygltf::Buffer input_buffer = model.buffers[input_view.buffer];

            tinygltf::Accessor output = model.accessors[sampler.output];
            tinygltf::BufferView output_view = model.bufferViews[output.bufferView];
            tinygltf::Buffer output_buffer = model.buffers[output_view.buffer];

            float * input_data = (float *)(input_buffer.data.data() + input_view.byteOffset + input.byteOffset);
            float * output_data = (float *)(output_buffer.data.data() + output_view.byteOffset + output.byteOffset);
            int id = -1;
            for(int i = 0; i < input.count - 1; ++i) {
                if(time >= input_data[i] && time < input_data[i + 1]) {
                    id = i;
                    break;
                }
            }
            if(id < 0)
                continue;

            if(channel.target_path == "rotation" && sampler.interpolation == "LINEAR") {
                float t = (time - input_data[id]) / (input_data[id + 1] - input_data[id]);
                node.rotation = glm::mix(glm::quat(float_to_vec4(output_data + 4 * id)), glm::quat(float_to_vec4(output_data + 4 * (id + 1))), t);
                continue;
            }

            std::function<float (float * ref, float v, float dest_a, float dest_b)> interpol;
            if(sampler.interpolation == "STEP")
                interpol = [](float * ref, float v, float dest_a, float dest_b) -> float {return dest_a;};
            else /* LINEAR (and CUBICSPLINE for the moment) */
                interpol = [](float * ref, float v, float dest_a, float dest_b) -> float {
                    float t = (v - ref[0]) / (ref[1] - ref[0]);
                    return dest_a * (1 - t) + dest_b * t;
                };

            int size;

            if(channel.target_path == "translation" || channel.target_path == "scale")
                size = 3;
            else if(channel.target_path == "rotation")
                size = 4;
            else
                size = 16;

            double * tmp = new double[size];

            for(int i = 0; i < size; ++i)
                tmp[i] = interpol(input_data + id, time, output_data[size * id + i], output_data[size * (id + 1) + i]);

            if(channel.target_path == "translation")
                node.translation = double_to_vec3(tmp);
            else if(channel.target_path == "scale")
                node.scale = double_to_vec3(tmp);
            else if(channel.target_path == "rotation")
                node.rotation = glm::quat(double_to_vec4(tmp));
            else
                node.matrix = double_to_mat4(tmp);

            delete [] tmp;
        }
    }
    computePosesTransforms();
}

void SceneDescriptor::drawVertices(int i, Deferred *renderer) {
	Shader *gShader = renderer->getGeometryShader();

	//std::cout << "draw " << i << std::endl;

	for (auto prim : model.meshes[i].primitives) {
		tinygltf::Accessor access = model.accessors[prim.indices];
		tinygltf::Material mat = model.materials[prim.material];

		glBindVertexArray(vaos[i]);

		for(int k = 0; k < 6; k++) {
			glActiveTexture(GL_TEXTURE0+k);
			glBindTexture(GL_TEXTURE_2D, 0);
		}


		glm::mat4 skinMatrix = (rig.find(i) != rig.end()) ? skin.poses[rig[i]].transform : glm::mat4(1);


		bool isDiffuse = false;
		if (activeTextures) {
			gShader->uniformMatrix("uSkinMatrix", skinMatrix);
			if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
				tinygltf::Value vals = mat.extensions["KHR_materials_pbrSpecularGlossiness"];
				const int index = vals.Get("diffuseTexture").Get("index").Get<int>();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[index]);
				isDiffuse = true;

				tinygltf::Value val3 = vals.Get("metallicRoughnessTexture");
				if (val3.IsObject()) {
					const int indexSpecular = val3.Get("index").Get<int>();
					glActiveTexture(GL_TEXTURE3);
					glBindTexture(GL_TEXTURE_2D, textures[indexSpecular]);
				}
			} else if (mat.values.find("baseColorTexture") != mat.values.end()) {
				const int index = mat.values["baseColorTexture"].TextureIndex();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, textures[index]);
				isDiffuse = true;
			}

			if(isDiffuse) {
				gShader->uniformVector("uKd", glm::vec3(-1, -1, -1));
			} else {
				if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end()) {
					tinygltf::Value vals = mat.extensions["KHR_materials_pbrSpecularGlossiness"];
					glm::vec3 kd = glm::vec3(vals.Get("diffuseFactor").Get(0).Get<double>(),
					vals.Get("diffuseFactor").Get(1).Get<double>(),vals.Get("diffuseFactor").Get(2).Get<double>());
					gShader->uniformVector("uKd", kd);
				} else if (mat.values.find("baseColorFactor") != mat.values.end()) {
					std::vector<double> v = mat.values["baseColorFactor"].number_array;
					glm::vec3 kd = glm::vec3(v[0], v[1], v[2]);
					gShader->uniformVector("uKd", kd);
				}
			}


			if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
				tinygltf::Parameter val2 = mat.additionalValues["emissiveTexture"];
				if (val2.TextureIndex() != -1) {
					int emissionIndex = val2.TextureIndex();
					glActiveTexture(GL_TEXTURE2);
					glBindTexture(GL_TEXTURE_2D, textures[emissionIndex]);
				}
			}

			if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
				tinygltf::Parameter val2 = mat.additionalValues["normalTexture"];
				if (val2.TextureIndex() != -1) {
					int normalIndex = val2.TextureIndex();
					glActiveTexture(GL_TEXTURE4);
					glBindTexture(GL_TEXTURE_2D, textures[normalIndex]);
				}
			}
			gShader->uniformValue("uShininess", 0.2f);
		} else {
			shadowShader->uniformMatrix("uSkinMatrix", skinMatrix);
		}
		glDrawElements(prim.mode, access.count, access.componentType,
					   nullptr);
		glBindVertexArray(0);
	}
}

void SceneDescriptor::drawNodes(tinygltf::Node &node, Deferred *renderer) {
	if (node.mesh != -1) {
	    drawVertices(node.mesh, renderer);
	}
	for (int i : node.children) {
		if (i != -1) {
			drawNodes(model.nodes[i], renderer);
		}
	}
}

void SceneDescriptor::drawModel(Deferred *renderer) {
	tinygltf::Scene defaultScene = model.scenes[0];
	for (int node : defaultScene.nodes) {
		drawNodes(model.nodes[node], renderer);
	}
}

void SceneDescriptor::debug() {
	std::cout << "|accessors| = " << model.accessors.size() << std::endl;
	std::cout << "|animations| = " << model.animations.size() << std::endl;
	std::cout << "|buffers| = " << model.buffers.size() << std::endl;
	for (int i = 0; i < model.buffers.size(); i++) {
		std::cout << "	|buffers[" << i << "]| = " << model.buffers[i].data.size() << " bytes" << std::endl;
	}
	std::cout << "|bufferViews| = " << model.bufferViews.size() << std::endl;
	std::cout << "|materials| = " << model.materials.size() << std::endl;
	std::cout << "|meshes| = " << model.meshes.size() << std::endl;
	for (int i = 0; i < model.meshes.size(); i++) {
		std::cout << "	|meshs[" << i << "]| = '" << model.meshes[i].name << "'" << model.meshes[i].primitives.size() << " primitives" << std::endl;
	}
	std::cout << "|nodes| = " << model.nodes.size() << std::endl;
	std::cout << "|textures| = " << model.textures.size() << std::endl;
	std::cout << "|images| = " << model.images.size() << std::endl;
	std::cout << "|skins| = " << model.skins.size() << std::endl;
	std::cout << "|samplers| = " << model.samplers.size() << std::endl;
	std::cout << "|cameras| = " << model.cameras.size() << std::endl;
	std::cout << "|scenes| = " << model.scenes.size() << std::endl;
	std::cout << "|lights| = " << model.lights.size() << std::endl;
	for (int i = 0; i < model.lights.size(); i++) {
		tinygltf::Light light = model.lights[i];
		std::cout << "	|lights[" << i << "]| = [";
		for (auto v: light.color) {
			std::cout << v << " ,";
		}
		std::cout << "]" << std::endl;
	}

	for(int i = 0; i < model.materials.size(); i++) {
		std::cout << "mat n°" << i << std::endl;
		tinygltf::Material mat = model.materials[i];
		for (auto v: mat.values) {
			std::cout << "\tMat Values : " << v.first << std::endl;
		}
		for (auto v: mat.additionalValues) {
			std::cout << "\tMat additionalValues : " << v.first << std::endl;
		}
		for (auto v: mat.extensions) {
			std::cout << "\tMat extensions : " << v.first << std::endl;
			tinygltf::Value val = v.second;
			auto vector = val.Keys();
			for (auto k : vector) {
				std::cout << "\t\t" << "key:" << k << std::endl;
			}
		}
	}

	for (auto v: model.extensions) {
		std::cout << "Model extensions : " << v.first << std::endl;
		tinygltf::Value val = v.second;
		auto vector = val.Keys();
		for (auto k : vector) {
			std::cout << "\t" << "key:" << k << std::endl;
		}
	}

	for (auto v: model.extensionsUsed) {
		std::cout << "extensions used : " << v << std::endl;
	}

	auto vector = model.extras.Keys();
	std::cout << "extra values :" << std::endl;
	for (auto k : vector) {
		std::cout << "\t" << "key:" << k << std::endl;
	}

	for (auto v: model.scenes[0].extensions) {
		std::cout << "scene extensions : " << v.first << std::endl;
	}


}

void SceneDescriptor::loadOneVBO(GLuint *vbo, tinygltf::Accessor access) {
	tinygltf::BufferView view = model.bufferViews[access.bufferView];
	tinygltf::Buffer buffer = model.buffers[view.buffer];
	glGenBuffers(1, vbo);
	glBindBuffer(view.target, *vbo);
	glBufferData(view.target, view.byteLength, buffer.data.data() + view.byteOffset + access.byteOffset,
				 GL_STATIC_DRAW);
	glBindBuffer(view.target, 0);
}

GLuint SceneDescriptor::loadMesh(tinygltf::Mesh &mesh) {
	GLuint vao;
	GLuint ibo;

	glGenVertexArrays(1, &vao);

	tinygltf::Primitive prim = mesh.primitives[0];

	GLuint vbo;
	tinygltf::Accessor access = model.accessors[prim.indices];
	loadOneVBO(&vbo, access);
	ibo = vbo;


	glBindVertexArray(vao);
	std::cout << "Binding IBO... accessor = " << prim.indices << " bufferView = "
			  << model.accessors[prim.indices].bufferView <<
			  "(size=" << model.bufferViews[model.accessors[prim.indices].bufferView].byteLength << " " << std::endl;
	for (auto &attrib : prim.attributes) {
		tinygltf::Accessor access = model.accessors[attrib.second];

		int target = -1;
		if (attrib.first == "POSITION") {
			target = 0;
		} else if (attrib.first == "NORMAL") {
			target = 1;
		} else if (attrib.first == "TEXCOORD_0") {
			target = 2;
		} else if (attrib.first == "TANGENT") {
			target = 3;
		} else {
			std::cout << "- Attribute " << attrib.first << " unsuported" << std::endl;
			continue;
		}

		GLuint vbo;
		loadOneVBO(&vbo, access);
		std::cout << "Attrib " << attrib.first << "... accessor = " << attrib.second << " bufferView = "
				  << access.bufferView << std::endl;
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(target);
		std::cout << "AttribPoint " << target << " " << access.type << " " << access.componentType << " "
				  << access.ByteStride(model.bufferViews[access.bufferView]) << std::endl;
		glVertexAttribPointer(target, access.type, access.componentType,
							  access.normalized ? GL_TRUE : GL_FALSE,
							  access.ByteStride(model.bufferViews[access.bufferView]), nullptr);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray(0);

	return vao;
}

void SceneDescriptor::bindScene() {
	std::cout << "Loading VAO..." << std::endl;
	for (int i = 0; i < model.meshes.size(); i++) {
		std::cout << i << "/" << model.meshes.size() << std::endl;
		GLuint vao = loadMesh(model.meshes[i]);
		vaos[i] = vao;
	}

	std::cout << "Loading Textures..." << std::endl;
	loadAllTextures();
    loadPoses();
}

void SceneDescriptor::computePosesTransforms() {
    std::queue<int> todo;
    todo.push(skin.root);
    while(! todo.empty()) {
        int i = todo.front();
        todo.pop();
        NodePose & node = skin.poses[i];
        node.computeLocalTransform();
        if(node.parent != -1)
            node.transform = skin.poses[node.parent].transform * node.transform;
        for(int c : model.nodes[i].children)
            todo.push(c);
    }
}

void SceneDescriptor::activeTexturing() {
	activeTextures = true;
}

void SceneDescriptor::desactiveTexturing() {
	activeTextures = false;
}

void SceneDescriptor::computeBox() {
	tinygltf::Primitive prim = model.meshes[0].primitives[0];
	int idAccess = -1;
	for (auto &attrib : prim.attributes) {
		if (attrib.first == "POSITION") {
			idAccess = attrib.second;
			break;
		}
	}
	tinygltf::Accessor access = model.accessors[idAccess];
	this->min = glm::vec3(access.minValues[0], access.minValues[1], access.minValues[2]);
	this->max = glm::vec3(access.maxValues[0], access.maxValues[1], access.maxValues[2]);
	this->center = (this->min + this->max) / glm::vec3(2, 2, 2);
}

const glm::vec3 SceneDescriptor::getMin() const {
	return min;
}

const glm::vec3 SceneDescriptor::getMax() const {
	return max;
}

const glm::vec3 SceneDescriptor::getCenter() const {
	return center;
}

const float SceneDescriptor::size() const {
	return std::max(std::abs(glm::distance(min, center)), std::abs(glm::distance(max, center)));
}

Skin & SceneDescriptor::animData() {
    return skin;
}


