#ifndef OPENGLNOEL_SKIN_H
#define OPENGLNOEL_SKIN_H

#include <vector>
#include <tiny_gltf.h>
#include "NodePose.h"

struct Skin {
    std::vector<NodePose> poses;
    int root;
    int nb_meshes;

    double last_time;
    bool do_anim;
    bool do_restart;
    float speed;
    double frame;

    Skin();
    void debug(tinygltf::Model & model);

    void refresh();
    void restart();
    void stop();
    void launch();
};

#endif //OPENGLNOEL_SKIN_H
