#include "archlib/target_system/Effector.hpp"

namespace arch {
	namespace target_system {

		Effector::Effector(int &argc, char **argv, const std::string &name) : ROSComponent(argc, argv, name) {}
		Effector::~Effector() {}

        void Effector::setUp() {}
        void Effector::tearDown() {}

    	int32_t Effector::run() {
			setUp();    
            ros::spin();			
			tearDown();

            return 0;
		}
	}
}