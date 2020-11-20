#pragma once

#ifndef TPENDPOINT_FILE_H
#define TPENDPOINT_FILE_H
#include <pjsua2.hpp>


namespace tp {
    class Endpoint : public pj::Endpoint
	{
        void onNatDetectionComplete(const OnNatDetectionCompleteParam &prm){ 
            PJ_UNUSED_ARG(prm); 
        }

        
    };
}

#endif
