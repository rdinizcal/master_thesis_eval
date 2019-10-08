#ifndef DATA_ACCESS_HPP
#define DATA_ACCESS_HPP

#include <fstream>
#include <chrono>
#include <deque>

#include "ros/ros.h"
#include <ros/package.h>

#include "bsn/operation/Operation.hpp"

#include "archlib/Persist.h"
#include "archlib/DataAccessRequest.h"
#include "archlib/ROSComponent.hpp"

#include "StatusMessage.hpp"
#include "EventMessage.hpp"
#include "UncertaintyMessage.hpp"
#include "AdaptationMessage.hpp"


class DataAccess : public arch::ROSComponent {

	public:
		DataAccess(int &argc, char **argv, const std::string &name);
		virtual ~DataAccess();

	private:
		DataAccess(const DataAccess &);
		DataAccess &operator=(const DataAccess &);
		int64_t now() const;

		void persistEvent(const int64_t &timestamp, const std::string &source, const std::string &target, const std::string &content);
		void persistStatus(const int64_t &timestamp, const std::string &source, const std::string &target, const std::string &content);
		void persistUncertainty(const int64_t &timestamp, const std::string &source, const std::string &target, const std::string &content);
		void persistAdaptation(const int64_t &timestamp, const std::string &source, const std::string &target, const std::string &content);

		void flush();

	public:
		virtual void setUp();
		virtual void tearDown();
		virtual void body();

		void receivePersistMessage(const archlib::Persist::ConstPtr& msg);
		bool processQuery(archlib::DataAccessRequest::Request &req, archlib::DataAccessRequest::Response &res);

	protected:
		ros::NodeHandle handle;
	
	private:
		std::fstream fp;
		std::string event_filepath;
		std::string status_filepath;
		std::string uncertainty_filepath;
		std::string adaptation_filepath;

		int64_t logical_clock;

		std::vector<StatusMessage> statusVec;
		std::vector<EventMessage> eventVec;
		std::vector<UncertaintyMessage> uncertainVec;
		std::vector<AdaptationMessage> adaptVec;

		std::map<std::string, std::deque<std::string>> status;
		std::map<std::string, std::deque<std::string>> events;
		int buffer_size;
};

#endif 