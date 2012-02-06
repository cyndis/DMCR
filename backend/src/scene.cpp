#include "scene.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include "vector.h"
#include "sceneobject.h"

dmcr::Scene::Scene()
{
}

int dmcr::Scene::loadFromFile(const std::string &file_name)
{
    std::ifstream input_file;
    input_file.open(file_name.c_str());
    if (!input_file.is_open()) {
        std::cout << "Failed to open file: " << file_name << std::endl;
        return -1;
    }

    std::string data((std::istreambuf_iterator<char>(input_file)),
                     std::istreambuf_iterator<char>());

    input_file.close();

    loadFromString(data);

    return 0;
}

int dmcr::Scene::loadFromString(const std::string &string)
{
    Json::Value root;
    Json::Reader reader;
    bool parsing_succesful = reader.parse(string, root);
    if (!parsing_succesful) {
        std::cout << "Failed to parse scene" << std::endl
                  << reader.getFormatedErrorMessages();
        return -1;
    }

    const Json::Value objects = root["scene"];
    for (unsigned int i = 0; i < objects.size(); ++i) {
        const Json::Value position = objects[i]["position"];
        Vector3f position_value;

        if (!position || !position.isArray() || position.size() != 3) {
            std::cout << "No position specified for object, assuming (0, 0, 0)"
                      << std::endl;
            position_value = Vector3f(0, 0, 0);
        } else {
            position_value = Vector3f(position[0u].asDouble(),
                                     position[1].asDouble(),
                                     position[2].asDouble());
        }

        /*
           The next section of code leaks memory if an exception is thrown
           before the derived object pointer is assigned to the SceneObjectPtr
           (which is quite unlikely)
        */
        std::string type = objects[i]["type"].asString();
        if (type == "sphere") {
            Sphere *sphere = new Sphere;

            const Json::Value radius = objects[i]["radius"];
            float radius_value = 1.0f;

            if (!radius)
                std::cout << "No radius specified for sphere, assuming 1"
                          << std::endl;
            else
                radius_value = radius.asDouble();

            sphere->setPosition(position_value);
            sphere->setRadius(radius_value);
            dmcr::SceneObjectPtr object(sphere);
            addObject(object);
        } else if (type == "box") {
            Box *box = new Box;
            box->setPosition(position_value);
            dmcr::SceneObjectPtr object(box);
            addObject(object);
        } else {
            std::cout << "Unknown object type: " << type << std::endl;
            continue;
        }
    }

    return 0;
}
