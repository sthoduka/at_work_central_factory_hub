/***************************************************************************
 *  triggered_conveyor_belt_thread.cpp - Thread to communicate with
 *                                                  the conveyor belt and
 *                                                  quality control camera.
 *
 *  Created: Wed Oct 28 11:47:42 2015
 *  Copyright  2015 Alexander Moriarty
 ****************************************************************************/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.GPL file in the doc directory.
 */

#include "triggered_conveyor_belt_thread.h"

#include <core/threading/mutex_locker.h>
#include <boost/lexical_cast.hpp>
#include <boost/thread/thread.hpp>

/** @class TriggeredConveyorBeltThread
 * Thread to communicate with the @Work conveyor belt and quality control
 * camera.
 *
 * @author Alexander Moriarty
 */

TriggeredConveyorBeltThread::TriggeredConveyorBeltThread() :
        Thread("TriggeredConveyorBeltThread", Thread::OPMODE_CONTINUOUS), zmq_context_(NULL), zmq_publisher_(NULL), zmq_subscriber_(NULL), cfg_timer_interval_(40), default_network_interface_("eth0")
{
  conveyor_mutex_ = new fawkes::Mutex();
  camera_mutex_ = new fawkes::Mutex();
}

TriggeredConveyorBeltThread::~TriggeredConveyorBeltThread(){
   delete conveyor_mutex_;
   delete camera_mutex_;
}

void TriggeredConveyorBeltThread::init()
{
    try
    {
        cfg_timer_interval_ = config->get_uint("/llsfrb/clips/timer-interval");
    } catch (fawkes::Exception &e)
    {
        // use default value
    }

    std::string host_ip_address = "";
    std::string host_command_port = "";
    std::string host_status_port = "";

    try {
        if (config->exists("/llsfrb/triggered-conveyor-belt/host")
            && config->exists("/llsfrb/triggered-conveyor-belt/command_port")
            && config->exists("/llsfrb/triggered-conveyor-belt/status_port"))
        {
            host_ip_address = config->get_string("/llsfrb/triggered-conveyor-belt/host");
            host_command_port = "epgm://" + default_network_interface_ + ":" + boost::lexical_cast<std::string>(config->get_uint("/llsfrb/triggered-conveyor-belt/command_port"));
            host_status_port = "epgm://" + host_ip_address + ":" + boost::lexical_cast<std::string>(config->get_uint("/llsfrb/triggered-conveyor-belt/status_port"));

            zmq_context_ = new zmq::context_t(1);
            int msg_limit = 1;

            // add publisher to send status messages
            logger->log_info("TriggeredConveyorBelt", "Connecting to the command port: %s", host_command_port.c_str());
            zmq_publisher_ = new zmq::socket_t(*zmq_context_, ZMQ_PUB);
            zmq_publisher_->setsockopt(ZMQ_CONFLATE, &msg_limit, sizeof(msg_limit));
            zmq_publisher_->bind(host_command_port.c_str());

            // add subscriber to receive command messages from a client
            logger->log_info("TriggeredConveyorBelt", "Connecting to the status port: %s", host_status_port.c_str());
            zmq_subscriber_ = new zmq::socket_t(*zmq_context_, ZMQ_SUB);
            zmq_subscriber_->setsockopt(ZMQ_SUBSCRIBE, "", 0);
            zmq_subscriber_->setsockopt(ZMQ_CONFLATE, &msg_limit, sizeof(msg_limit));
            zmq_subscriber_->connect(host_status_port.c_str());
        }
    } catch (std::exception &e)
    {
        logger->log_warn("TriggeredConveyorBelt", "Cannot establish communication with the conveyor belt: %s", e.what());

        if (zmq_context_ != NULL) {
          delete zmq_context_;
          zmq_context_ = NULL;
        }
        if (zmq_publisher_ != NULL) {
          delete zmq_publisher_;
          zmq_publisher_ = NULL;
        }
        if (zmq_subscriber_ != NULL) {
          delete zmq_subscriber_;
          zmq_subscriber_ = NULL;
        }
    }

    fawkes::MutexLocker lock(clips_mutex);

    clips->add_function("conveyor-belt-start-belt", sigc::slot<void>(sigc::mem_fun(*this, &TriggeredConveyorBeltThread::clips_start_belt)));
    clips->add_function("conveyor-belt-stop-belt", sigc::slot<void>(sigc::mem_fun(*this, &TriggeredConveyorBeltThread::clips_stop_belt)));
    clips->add_function("conveyor-belt-is-running", sigc::slot<bool>(sigc::mem_fun(*this, &TriggeredConveyorBeltThread::clips_is_belt_running)));
    clips->add_function("conveyor-belt-is-device-connected", sigc::slot<bool>(sigc::mem_fun(*this, &TriggeredConveyorBeltThread::clips_is_belt_connected)));

    if (!clips->build("(deffacts have-feature-conveyor-belt (have-feature ConveyorBelt))"))
        logger->log_warn("TriggeredConveyorBelt", "Failed to build deffacts have-feature-conveyor-belt");


    clips->add_function("quality-control-camera-is-device-connected", sigc::slot<bool>(sigc::mem_fun(*this, &TriggeredConveyorBeltThread::clips_is_camera_connected)));

    if (!clips->build("(deffacts have-feature-quality-control-camera (have-feature QualityControlCamera))"))
        logger->log_warn("TriggeredConveyorBelt", "Failed to build deffacts have-feature-quality-control-camera");
}

void TriggeredConveyorBeltThread::finalize()
{
  if (zmq_context_ != NULL) {
    delete zmq_context_;
    zmq_context_ = NULL;
  }
  if (zmq_publisher_ != NULL) {
    delete zmq_publisher_;
    zmq_publisher_ = NULL;
  }
  if (zmq_subscriber_ != NULL) {
    delete zmq_subscriber_;
    zmq_subscriber_ = NULL;
  }
}

void TriggeredConveyorBeltThread::loop()
{
    receiveAndBufferStatusMsgs();
    // unconditional sleep
    boost::this_thread::sleep(boost::posix_time::milliseconds(cfg_timer_interval_));

}

bool TriggeredConveyorBeltThread::clips_is_belt_running()
{
    //fawkes::MutexLocker ml(conveyor_mutex_);

    if (last_conveyor_status_msg_.has_mode()
        && last_conveyor_status_msg_.mode() == START)
    {
        return true;
    } else
        return false;
}

bool TriggeredConveyorBeltThread::clips_is_belt_connected()
{
    //fawkes::MutexLocker ml(conveyor_mutex_);

    if (last_conveyor_status_msg_.has_is_device_connected()
        && last_conveyor_status_msg_.is_device_connected())
    {
        return true;
    } else
        return false;
}

bool TriggeredConveyorBeltThread::clips_is_camera_connected()
{
    //f/awkes::MutexLocker ml(camera_mutex_);

    if (last_camera_status_msg_.has_is_device_connected()
        && last_camera_status_msg_.is_device_connected())
    {
        return true;
    } else
        return false;
}

void TriggeredConveyorBeltThread::clips_start_belt()
{
    setConveyorBeltRunMode(START);
}

void TriggeredConveyorBeltThread::clips_stop_belt()
{
    setConveyorBeltRunMode(STOP);
}

void TriggeredConveyorBeltThread::setConveyorBeltRunMode(RunMode mode)
{
    //fawkes::MutexLocker ml(conveyor_mutex_);
    boost::posix_time::time_duration time_diff;

    // prevent sending to many messages to the device
    time_diff = boost::posix_time::microsec_clock::local_time()
                - last_sent_command_timestamp_;
    if (time_diff.total_milliseconds() < 1000)
        return;

    ConveyorBeltCommand command_msg;
    std::string serialized_string;

    command_msg.set_mode(mode);

    zmq::message_t *query = NULL;
    try
    {
        command_msg.SerializeToString(&serialized_string);
        query = new zmq::message_t(serialized_string.length());
        memcpy(query->data(), serialized_string.c_str(), serialized_string.length());
        zmq_publisher_->send(*query);

        last_sent_command_timestamp_ = boost::posix_time::microsec_clock::local_time();

        logger->log_info("TriggeredConveyorBelt", "Set run mode: %d", command_msg.mode());

        delete query;
    } catch (fawkes::Exception &e)
    {
        logger->log_warn("TriggeredConveyorBelt", "Failed to set run mode: %s", e.what());

        if (query != NULL)
            delete query;
    }

}

void TriggeredConveyorBeltThread::receiveAndBufferStatusMsgs()
{
    //fawkes::MutexLocker camera_lock(camera_mutex_);
    //fawkes::MutexLocker conveyor_lock(conveyor_mutex_);
    fawkes::MutexLocker clips_lock(clips_mutex);
    if (zmq_subscriber_->recv(&zmq_incomming_msg_, ZMQ_NOBLOCK))
    {
        if(last_camera_status_msg_.ParseFromArray(zmq_incomming_msg_.data(),
                                                  zmq_incomming_msg_.size()))
        {

          if(last_camera_status_msg_.has_state()){
          // remember time of last received msg
          prev_camera_update_timestamp_ = boost::posix_time::microsec_clock::local_time();

          //fawkes::MutexLocker ml(camera_mutex_);
          QualityControlCameraStatus::State detected_plate;
          detected_plate = last_camera_status_msg_.state();
          logger->log_warn("TriggeredConveyorBelt", "DETECTED PLATE %d", detected_plate);
          // let CLIPS know about the plate by asserting it as a fact
          std::stringstream sstr;
          sstr << "(quality-control-camera-object " << detected_plate << ")";


          fawkes::MutexLocker lock(clips_mutex);
          clips->assert_fact(sstr.str());
          }
        }
        if (last_conveyor_status_msg_.ParseFromArray(zmq_incomming_msg_.data(),
                                                     zmq_incomming_msg_.size()))
        {

          logger->log_warn("TriggeredConveyorBelt", "SANTOSH 2");

            prev_conveyor_update_timestamp_ = boost::posix_time::microsec_clock::local_time();
        }

    }
    boost::posix_time::time_duration camera_time_diff = boost::posix_time::microsec_clock::local_time()
                                                  - prev_camera_update_timestamp_;
    if (camera_time_diff.total_seconds() >= 3)
    {
        last_camera_status_msg_.set_is_device_connected(false);
        //last_camera_status_msg_.set_state(QualityControlCameraStatus::UNKNOWN_PLATE);
    }

    boost::posix_time::time_duration conveyor_time_diff = boost::posix_time::microsec_clock::local_time()
                                                  - prev_conveyor_update_timestamp_;
    if (conveyor_time_diff.total_seconds() >= 3)
    {
        last_conveyor_status_msg_.set_is_device_connected(false);
        last_conveyor_status_msg_.set_mode(STOP);
    }
}
