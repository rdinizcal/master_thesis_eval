#include "Logger.hpp"

Logger::Logger(int  &argc, char **argv, std::string name) : fp(), filepath(), logical_clock(0) {}
Logger::~Logger() {}

std::string Logger::now() const{
    return std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

void Logger::setUp() {
    std::string path = ros::package::getPath("logger");
    std::string now = this->now();

    filepath = path + "/resource/logs/" + now + ".log";
    std::cout << filepath;

    fp.open(filepath, std::fstream::in | std::fstream::out | std::fstream::trunc);
    fp << "\n";
    fp.close();

    ros::NodeHandle handler;
    status_logger2manager_pub = handler.advertise<messages::Status>("status", 10);
    event_logger2manager_pub = handler.advertise<messages::Event>("event", 10);
}

void Logger::receiveStatus(const messages::Status::ConstPtr& msg) {
    //ROS_INFO("I heard: [%s]", msg->task_id.c_str());
    ++logical_clock;

    // persist
    fp.open(filepath, std::fstream::in | std::fstream::out | std::fstream::app);
    fp << logical_clock << ",";
    fp << this->now() << ",";
    fp << msg->key << ",";
    fp << msg->value << "\n";

    fp.close();

    //forward
    status_logger2manager_pub.publish(msg);
}

void Logger::receiveEvent(const messages::Event::ConstPtr& msg) {
    //ROS_INFO("I heard: [%s]", msg->task_id.c_str());
    ++logical_clock;

    // persist
    fp.open(filepath, std::fstream::in | std::fstream::out | std::fstream::app);
    fp << logical_clock << ",";
    fp << this->now() << ",";
    fp << msg->type << ",";
    fp << msg->description << "\n";

    fp.close();

    //forward
    event_logger2manager_pub.publish(msg);
}

void Logger::run(){
    ros::NodeHandle n;
    ros::Subscriber status_sub = n.subscribe("log_status", 1000, &Logger::receiveStatus, this);
    ros::Subscriber event_sub = n.subscribe("log_event", 1000, &Logger::receiveEvent, this);
    
    ros::spin();
}
