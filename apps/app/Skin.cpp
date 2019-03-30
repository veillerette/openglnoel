#include "Skin.h"

#include <iostream>
#include <glmlv/GLFWHandle.hpp>

Skin::Skin() {
    root = -1;
    nb_meshes = 0;
    last_time = glfwGetTime();
    do_anim = false;
    do_restart = false;
    speed = 1;
}

static void print_mat4(std::ostream & stream, glm::mat4 & in_mat) {
    glm::mat4 mat = glm::transpose(in_mat);
    for(int i = 0; i < 4; ++i)
        stream << mat[i].x << " " << mat[i].y << " "  << mat[i].z << " "  << mat[i].w << std::endl;
}

void Skin::debug(tinygltf::Model & model) {
    std::cout << "root : " << root << std::endl;
    for(int i = 0; i < poses.size(); ++i) {
        NodePose & node = poses[i];
        std::cout << "node :" << i << std::endl;
        std::cout << "children : [";
        for(int c : model.nodes[i].children)
            std::cout << c << ", ";
        std::cout << "]" << std::endl;
        print_mat4(std::cout, node.transform);
    }
}

void Skin::refresh() {
    if(do_restart) {
        do_restart = false;
        restart();
    }
    if(do_anim) {
        frame += (glfwGetTime() - last_time) * speed;
    }
    last_time = glfwGetTime();
}

void Skin::restart() {
    last_time = glfwGetTime();
    frame = 0;
    for(NodePose & node : poses)
        node.reset();
}

void Skin::stop() {
    do_anim = false;
}

void Skin::launch() {
    do_anim = true;
}
