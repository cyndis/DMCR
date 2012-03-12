#include "../src/taskmanager.h"
#include "../src/task.h"
#include "../src/scene.h"
#include "../src/dummyscene.h"
#include "../src/octreescene.h"
#include "../src/itaskprovider.h"
#include <unistd.h>
#include <string>
#include <iostream>

class MyTaskProvider : public dmcr::ITaskProvider {
public:
    MyTaskProvider() {
        completed = false;
    }
    
    bool completed;
    
    virtual void onTaskCompleted(uint32_t /*task_id*/, 
                                 const dmcr::RenderResultPtr& /*result*/)
    {
        completed = true;
    }
};

void test_tasks() {
    dmcr::SceneFactory factory = [](const std::string& /*str*/) {
        auto scene = std::make_shared<dmcr::OctreeScene>();
        scene->loadFromFile("assets/scene.json");
        return scene;
    };
    
    dmcr::TaskManager manager(factory);
    
    MyTaskProvider provider;
    
    manager.onNewTask(&provider, 1, 640, 480, 1, "");
    
    for (int secs = 5; secs >= 0; --secs) {
        if (provider.completed)
            return;
        sleep(1);
    }
    
    throw std::runtime_error("Task was not completed in five seconds");
}
