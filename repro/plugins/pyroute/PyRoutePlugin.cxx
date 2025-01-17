
// NOTE: Python.h must be included before any standard headers
// See: https://bugzilla.redhat.com/show_bug.cgi?id=518385

/* Using the PyCXX API for C++ Python integration
 * It is extremely convenient and avoids the need to write boilerplate
 * code for handling the Python reference counts.
 * It is licensed under BSD terms compatible with reSIProcate */
#include <Python.h>
#include <CXX/Objects.hxx>
#include <CXX/Extensions.hxx>

#include <memory>
#include <utility>

#include "rutil/Logger.hxx"
#include "resip/stack/Helper.hxx"
#include "resip/stack/Dispatcher.hxx"
#include "repro/Plugin.hxx"
#include "repro/Processor.hxx"
#include "repro/Proxy.hxx"
#include "repro/RequestContext.hxx"
#include "repro/monkeys/LocationServer.hxx"

#include "rutil/PyExtensionBase.hxx"

#include "PyRouteWorker.hxx"
#include "PyRouteProcessor.hxx"

#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

using namespace resip;
using namespace repro;

class PyRoutePlugin : public repro::Plugin, public PyExtensionBase
{
   public:
      PyRoutePlugin() : PyExtensionBase("resip")
      {
      };

      virtual ~PyRoutePlugin()
      {
      };

      virtual bool onStartup() override
      {
         std::unique_ptr<Py::Module> pyModule = loadModule(mRouteScript);
         if(!pyModule)
         {
            return false;
         }

         if(pyModule->getDict().hasKey("on_load"))
         {
            DebugLog(<< "found on_load method, trying to invoke it...");
            try
            {
               StackLog(<< "invoking on_load");
               pyModule->callMemberFunction("on_load");
            }
            catch (const Py::Exception& ex)
            {
               DebugLog(<< "call to on_load method failed: " << Py::value(ex));
               StackLog(<< Py::trace(ex));
               return false;
            }
         }

         mPyModule = std::move(pyModule);

         mAction = mPyModule->getAttr("provide_route");

         return true;
      }


      virtual bool init(SipStack& sipStack, ProxyConfig *proxyConfig)
      {
         DebugLog(<<"PyRoutePlugin: init called");

         if(!proxyConfig)
         {
            ErrLog(<<"proxyConfig == 0, aborting");
            return false;
         }

         Data pyPath(proxyConfig->getConfigData("PyRoutePath", "", true));
         mRouteScript = proxyConfig->getConfigData("PyRouteScript", "", true);
         if(pyPath.empty())
         {
            ErrLog(<<"PyRoutePath not specified in config, aborting");
            return false;
         }
         if(mRouteScript.empty())
         {
            ErrLog(<<"PyRouteScript not specified in config, aborting");
            return false;
         }

         if(!PyExtensionBase::init(pyPath))
         {
            return false;
         }

         int numPyRouteWorkerThreads = proxyConfig->getConfigInt("PyRouteNumWorkerThreads", 2);
         std::unique_ptr<Worker> worker(new PyRouteWorker(*this, mAction));
         mDispatcher.reset(new Dispatcher(std::move(worker), &sipStack, numPyRouteWorkerThreads));

         return true;
      }

      virtual void onRequestProcessorChainPopulated(ProcessorChain& chain)
      {
         DebugLog(<<"PyRoutePlugin: onRequestProcessorChainPopulated called");

         // The module class is also the monkey class, no need to create
         // any monkey instance here

         // Add the pyroute monkey to the chain ahead of LocationServer
         std::unique_ptr<Processor> proc(new PyRouteProcessor(*mDispatcher));
         chain.insertProcessor<LocationServer>(std::move(proc));
      }

      virtual void onResponseProcessorChainPopulated(ProcessorChain& chain)
      {
         DebugLog(<<"PyRoutePlugin: onResponseProcessorChainPopulated called");
      }

      virtual void onTargetProcessorChainPopulated(ProcessorChain& chain)
      {
         DebugLog(<<"PyRoutePlugin: onTargetProcessorChainPopulated called");
      }

      virtual void onReload() 
      {
         DebugLog(<<"PyRoutePlugin: onReload called");
      }

   private:
      Data mRouteScript;
      std::unique_ptr<Py::Module> mPyModule;
      Py::Callable mAction;
      std::unique_ptr<Dispatcher> mDispatcher;
};


extern "C" {

static
std::shared_ptr<resip::Plugin> instantiate()
{
   return std::make_shared<PyRoutePlugin>();
}

PluginDescriptor reproPluginDesc =
{
   REPRO_DSO_PLUGIN_API_VERSION,
   &instantiate
};

};

/* ====================================================================
 *
 * Copyright (c) 2022, Software Freedom Institute https://softwarefreedom.institute
 * Copyright (c) 2013-2022, Daniel Pocock https://danielpocock.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. Neither the name of the author(s) nor the names of any contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * ====================================================================
 *
 *
 */

