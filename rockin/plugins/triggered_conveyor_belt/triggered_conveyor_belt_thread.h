/***************************************************************************
 *  triggered_conveyor_belt_thread.h - Thread to communicate with the
 *                                                conveyor belt and quality
 *                                                control camera.
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

#ifndef __PLUGINS_TRIGGERED_CONVEYOR_BELT_THREAD_H_
#define __PLUGINS_TRIGGERED_CONVEYOR_BELT_THREAD_H_

#include <core/threading/thread.h>
#include <aspect/logging.h>
#include <aspect/clips.h>
#include <aspect/configurable.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <plugins/msgs/DeviceConveyorBelt.pb.h>
#include <plugins/msgs/DeviceQualityControlCamera.pb.h>
#include <zmq.hpp>

class TriggeredConveyorBeltThread : public fawkes::Thread, public fawkes::LoggingAspect, public fawkes::ConfigurableAspect, public fawkes::CLIPSAspect
{
    public:
        TriggeredConveyorBeltThread();

        virtual void init();
        virtual void loop();
        virtual void finalize();

    private:
        void clips_start_belt();
        void clips_stop_belt();
        bool clips_is_belt_running();
        bool clips_is_belt_connected();
        bool clips_is_camera_connected();

        void setConveyorBeltRunMode(RunMode runmode);
        void receiveAndBufferStatusMsgs();

    private:
        zmq::context_t *zmq_context_;
        zmq::socket_t *zmq_publisher_;
        zmq::socket_t *zmq_subscriber_;
        unsigned int cfg_timer_interval_;

        std::string default_network_interface_;

        fawkes::Mutex *conveyor_mutex_;
        fawkes::Mutex *camera_mutex_;

        ConveyorBeltStatus last_conveyor_status_msg_;
        QualityControlCameraStatus last_camera_status_msg_;

        zmq::message_t zmq_incomming_msg_;
//      zmq::message_t zmq_camera_msg_;

        boost::posix_time::ptime prev_conveyor_update_timestamp_;
        boost::posix_time::ptime prev_camera_update_timestamp_;
        boost::posix_time::ptime last_sent_command_timestamp_;
};

#endif
