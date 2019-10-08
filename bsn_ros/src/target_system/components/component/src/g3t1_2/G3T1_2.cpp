#include "component/g3t1_2/G3T1_2.hpp"

#define BATT_UNIT 0.01

using namespace bsn::range;
using namespace bsn::generator;
using namespace bsn::operation;
using namespace bsn::configuration;

G3T1_2::G3T1_2(int &argc, char **argv, const std::string &name) :
    Sensor(argc, argv, name, "ecg", true, 1, bsn::resource::Battery("ecg_batt", 100, 100, 1)),
    markov(),
    dataGenerator(),
    filter(1),
    sensorConfig(),
    collected_risk() {}

G3T1_2::~G3T1_2() {}

void G3T1_2::setUp() {
    Component::setUp();
    
    srand(time(NULL));
        
    Operation op;
    
    std::vector<std::string> t_probs;
    std::array<float, 25> transitions;
    std::array<bsn::range::Range,5> ranges;
    std::string s;

    for(uint32_t i = 0; i < transitions.size(); i++){
        for(uint32_t j = 0; j < 5; j++){
            handle.getParam("state" + std::to_string(j), s);
            t_probs = op.split(s, ',');
            for(uint32_t k = 0; k < 5; k++){
                transitions[i++] = std::stod(t_probs[k]);
            }
        }
    }
    
    { // Configure markov chain
        std::vector<std::string> lrs,mrs0,hrs0,mrs1,hrs1;

        handle.getParam("LowRisk", s);
        lrs = op.split(s, ',');
        handle.getParam("MidRisk0", s);
        mrs0 = op.split(s, ',');
        handle.getParam("HighRisk0", s);
        hrs0 = op.split(s, ',');
        handle.getParam("MidRisk1", s);
        mrs1 = op.split(s, ',');
        handle.getParam("HighRisk1", s);
        hrs1 = op.split(s, ',');

        ranges[0] = Range(std::stod(hrs0[0]), std::stod(hrs0[1]));
        ranges[1] = Range(std::stod(mrs0[0]), std::stod(mrs0[1]));
        ranges[2] = Range(std::stod(lrs[0]), std::stod(lrs[1]));
        ranges[3] = Range(std::stod(mrs1[0]), std::stod(mrs1[1]));
        ranges[4] = Range(std::stod(hrs1[0]), std::stod(hrs1[1]));

        markov = Markov(transitions, ranges, 2);
        bsn::generator::DataGenerator dt(markov);
        dataGenerator = dt;
        dataGenerator.setSeed();
    }

    { // Configure sensor configuration
        Range low_range = ranges[2];
        
        std::array<Range,2> midRanges;
        midRanges[0] = ranges[1];
        midRanges[1] = ranges[3];
        
        std::array<Range,2> highRanges;
        highRanges[0] = ranges[0];
        highRanges[1] = ranges[4];

        std::array<Range,3> percentages;

        handle.getParam("lowrisk", s);
        std::vector<std::string> low_p = op.split(s, ',');
        percentages[0] = Range(std::stod(low_p[0]), std::stod(low_p[1]));

        handle.getParam("midrisk", s);
        std::vector<std::string> mid_p = op.split(s, ',');
        percentages[1] = Range(std::stod(mid_p[0]), std::stod(mid_p[1]));

        handle.getParam("highrisk", s);
        std::vector<std::string> high_p = op.split(s, ',');
        percentages[2] = Range(std::stod(high_p[0]), std::stod(high_p[1]));

        sensorConfig = SensorConfiguration(0, low_range, midRanges, highRanges, percentages);
    }

}

void G3T1_2::tearDown() {
    Component::tearDown();
}

double G3T1_2::collect() {
    double m_data = 0;

    m_data = dataGenerator.getValue();
    //battery.consume(BATT_UNIT);

    ROS_INFO("new data collected: [%s]", std::to_string(m_data).c_str());

    collected_risk = sensorConfig.evaluateNumber(m_data);

    return m_data;
}

double G3T1_2::process(const double &m_data) {
    double filtered_data;
    
    
    filter.insert(m_data);
    filtered_data = filter.getValue();
    //battery.consume(BATT_UNIT*filter.getRange());

    ROS_INFO("filtered data: [%s]", std::to_string(filtered_data).c_str());
    return filtered_data;
}

void G3T1_2::transfer(const double &m_data) {
    double risk;
    risk = sensorConfig.evaluateNumber(m_data);
    //battery.consume(BATT_UNIT);
    if (risk < 0 || risk > 100) throw std::domain_error("risk data out of boundaries");
    if (label(risk) != label(collected_risk)) throw std::domain_error("sensor accuracy fail");

    ros::NodeHandle handle;
    data_pub = handle.advertise<messages::SensorData>("ecg_data", 10);
    messages::SensorData msg;
    msg.type = type;
    msg.data = m_data;
    msg.risk = risk;
    msg.batt = battery.getCurrentLevel();

    data_pub.publish(msg);
    
    //battery.consume(BATT_UNIT);

    ROS_INFO("risk calculated and transferred: [%.2f%%]", risk);
    
}

std::string G3T1_2::label(double &risk) {
    std::string ans;
    if(sensorConfig.isLowRisk(risk)){
        ans = "low";
    } else if (sensorConfig.isMediumRisk(risk)) {
        ans = "moderate";
    } else if (sensorConfig.isHighRisk(risk)) {
        ans = "high";
    } else {
        ans = "unknown";
    }

    return ans;
}