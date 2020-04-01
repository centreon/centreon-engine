/*
 * Copyright 2020 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#include <gtest/gtest.h>
#include <time.h>
#include <cstring>
#include <iostream>
#include <memory>
#include "../test_engine.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/clib.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/configuration/applier/serviceescalation.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/serviceescalation.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ServiceRetention : public TestEngine {
 public:
  void SetUp() override {
    if (!config)
      config = new configuration::state;

    config->contacts().clear();
    configuration::applier::contact ct_aply;
    configuration::contact ctct{new_configuration_contact("admin", true)};
    ct_aply.add_object(ctct);
    ct_aply.expand_objects(*config);
    ct_aply.resolve_object(ctct);

    configuration::host hst{new_configuration_host("test_host", "admin")};
    configuration::applier::host hst_aply;
    hst_aply.add_object(hst);

    configuration::service svc{
        new_configuration_service("test_host", "test_svc", "admin")};
    configuration::applier::service svc_aply;
    svc_aply.add_object(svc);

    hst_aply.resolve_object(hst);
    svc_aply.resolve_object(svc);

    host_map const& hm{engine::host::hosts};
    _host = hm.begin()->second;
    _host->set_current_state(engine::host::state_up);
    _host->set_state_type(checkable::hard);
    _host->set_problem_has_been_acknowledged(false);
    _host->set_notify_on(static_cast<uint32_t>(-1));

    service_map const& sm{engine::service::services};
    _svc = sm.begin()->second;
    _svc->set_current_state(engine::service::state_ok);
    _svc->set_state_type(checkable::hard);
    _svc->set_problem_has_been_acknowledged(false);
    _svc->set_notify_on(static_cast<uint32_t>(-1));
  }

  void TearDown() override {
    deinit_config_state();
  }

 protected:
  std::shared_ptr<engine::host> _host;
  std::shared_ptr<engine::service> _svc;
};

TEST_F(ServiceRetention, RetentionWithMultilineOutput) {
  std::ostringstream oss;
  set_time(55000);

  time_t now = std::time(nullptr);
  oss.str("");
  oss << '[' << now << ']'
      << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;0;"
      "OK: Response time 0.123s | 'time'=0.123s;0:3;0:5;0; 'size'=81439B;;;0;\n"
    "<!DOCTYPE html><html lang=\"XXXXXX\"><head><meta charset=\"XXXXXX\"><meta http-equiv=\"XXXXXX\" content=\"XXXXXX\"><meta name=\"XXXXXX\" content=\"XXXXXX\"><title>Kibana</title><style>/* INTER UI FONT */\n"
    "/* INTER UI FONT */\n"
    "/* INTER UI FONT */\n"
    "/* INTER UI FONT */\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  normal;\n"
    "  font-weight: 100;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  italic;\n"
    "  font-weight: 100;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  normal;\n"
    "  font-weight: 200;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  italic;\n"
    "  font-weight: 200;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  normal;\n"
    "  font-weight: 300;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  italic;\n"
    "  font-weight: 300;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  normal;\n"
    "  font-weight: 400;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  italic;\n"
    "  font-weight: 400;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  normal;\n"
    "  font-weight: 500;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  italic;\n"
    "  font-weight: 500;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  normal;\n"
    "  font-weight: 600;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  italic;\n"
    "  font-weight: 600;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  normal;\n"
    "  font-weight: 700;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  italic;\n"
    "  font-weight: 700;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  normal;\n"
    "  font-weight: 800;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  italic;\n"
    "  font-weight: 800;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  normal;\n"
    "  font-weight: 900;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI';\n"
    "  font-style:  italic;\n"
    "  font-weight: 900;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "/* --------------------------------------------------------------------------\n"
    "Single variable font.\n"
    "\n"
    "Note that you may want to do something like this to make sure you're serving\n"
    "constant fonts to older browsers:\n"
    "html {\n"
    "  font-family: 'Inter UI', sans-serif;\n"
    "}\n"
    "@supports (font-variation-settings: normal) {\n"
    "  html {\n"
    "    font-family: 'Inter UI var', sans-serif;\n"
    "  }\n"
    "}\n"
    "\n"
    "BUGS:\n"
    "- Safari 12.0 will default to italic instead of regular when font-weight\n"
    "  is provided in a @font-face declaration.\n"
    "  Workaround: Use \"XXXXXX\" for Safari, or explicitly set\n"
    "  `font-variation-settings:\"XXXXXX\" DEGREE`.\n"
    "@font-face {\n"
    "  font-family: 'Inter UI var';\n"
    "  font-weight: 100 900;\n"
    "  font-style: oblique 0deg 10deg;\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "\n"
    "\"XXXXXX\" is recommended for Safari and Edge, for reliable italics.\n"
    "\n"
    "@supports (font-variation-settings: normal) {\n"
    "  html {\n"
    "    font-family: 'Inter UI var alt', sans-serif;\n"
    "  }\n"
    "}\n"
    "\n"
    "@font-face {\n"
    "  font-family: 'Inter UI var alt';\n"
    "  font-weight: 100 900;\n"
    "  font-style: normal;\n"
    "  font-named-instance: 'Regular';\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Inter UI var alt';\n"
    "  font-weight: 100 900;\n"
    "  font-style: italic;\n"
    "  font-named-instance: 'Italic';\n"
    "  src: url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\"),\n"
    "      url(\"XXXXXX\") format(\"XXXXXX\");\n"
    "}\n"
    "*/\n"
    "\n"
    "/* ROBOTO MONO FONTS */\n"
    "/* ROBOTO MONO FONTS */\n"
    "/* ROBOTO MONO FONTS */\n"
    "/* ROBOTO MONO FONTS */\n"
    "/* ROBOTO MONO FONTS */\n"
    "@font-face {\n"
    "  font-family: 'Roboto Mono';\n"
    "  font-style: italic;\n"
    "  font-weight: 400;\n"
    "  src: local('Roboto Mono Italic'), local('RobotoMono-Italic'), url(\"XXXXXX\") format('ttf');\n"
    "  unicode-range: U+0000-00FF, U+0131, U+0152-0153, U+02BB-02BC, U+02C6, U+02DA, U+02DC, U+2000-206F, U+2074, U+20AC, U+2122, U+2191, U+2193, U+2212, U+2215, U+FEFF, U+FFFD;\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Roboto Mono';\n"
    "  font-style: italic;\n"
    "  font-weight: 700;\n"
    "  src: local('Roboto Mono Bold Italic'), local('RobotoMono-BoldItalic'), url(\"XXXXXX\") format('woff2');\n"
    "  unicode-range: U+0000-00FF, U+0131, U+0152-0153, U+02BB-02BC, U+02C6, U+02DA, U+02DC, U+2000-206F, U+2074, U+20AC, U+2122, U+2191, U+2193, U+2212, U+2215, U+FEFF, U+FFFD;\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Roboto Mono';\n"
    "  font-style: normal;\n"
    "  font-weight: 400;\n"
    "  src: local('Roboto Mono'), local('RobotoMono-Regular'), url(\"XXXXXX\") format('woff2');\n"
    "  unicode-range: U+0000-00FF, U+0131, U+0152-0153, U+02BB-02BC, U+02C6, U+02DA, U+02DC, U+2000-206F, U+2074, U+20AC, U+2122, U+2191, U+2193, U+2212, U+2215, U+FEFF, U+FFFD;\n"
    "}\n"
    "@font-face {\n"
    "  font-family: 'Roboto Mono';\n"
    "  font-style: normal;\n"
    "  font-weight: 700;\n"
    "  src: local('Roboto Mono Bold'), local('RobotoMono-Bold'), url(\"XXXXXX\") format('woff2');\n"
    "  unicode-range: U+0000-00FF, U+0131, U+0152-0153, U+02BB-02BC, U+02C6, U+02DA, U+02DC, U+2000-206F, U+2074, U+20AC, U+2122, U+2191, U+2193, U+2212, U+2215, U+FEFF, U+FFFD;\n"
    "}</style><link rel=\"XXXXXX\" sizes=\"XXXXXX\" href=\"XXXXXX\"><link rel=\"XXXXXX\" type=\"XXXXXX\" href=\"XXXXXX\" sizes=\"XXXXXX\"><link rel=\"XXXXXX\" type=\"XXXXXX\" href=\"XXXXXX\" sizes=\"XXXXXX\"><link rel=\"XXXXXX\" href=\"XXXXXX\"><link rel=\"XXXXXX\" href=\"XXXXXX\" color=\"XXXXXX\"><link rel=\"XXXXXX\" href=\"XXXXXX\"><meta name=\"XXXXXX\" content=\"XXXXXX\"><meta name=\"XXXXXX\" content=\"XXXXXX\"><style>.kibanaWelcomeView {\n"
    "  height: 100%;\n"
    "  display: -webkit-box;\n"
    "  display: -webkit-flex;\n"
    "  display: -ms-flexbox;\n"
    "  display: flex;\n"
    "  -webkit-box-flex: 1;\n"
    "  -webkit-flex: 1 0 auto;\n"
    "      -ms-flex: 1 0 auto;\n"
    "          flex: 1 0 auto;\n"
    "  -webkit-box-orient: vertical;\n"
    "  -webkit-box-direction: normal;\n"
    "  -webkit-flex-direction: column;\n"
    "      -ms-flex-direction: column;\n"
    "          flex-direction: column;\n"
    "  -webkit-box-align: center;\n"
    "  -webkit-align-items: center;\n"
    "      -ms-flex-align: center;\n"
    "          align-items: center;\n"
    "  -webkit-box-pack: center;\n"
    "  -webkit-justify-content: center;\n"
    "      -ms-flex-pack: center;\n"
    "          justify-content: center;\n"
    "  background: #FFFFFF;\n"
    "}\n"
    "\n"
    ".kibanaWelcomeLogo {\n"
    "  width: 100%;\n"
    "  height: 100%;\n"
    "  background-repeat: no-repeat;\n"
    "  background-size: contain;\n"
    "  /* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX */\n"
    "  background-image: url(\"XXXXXX\");\n"
    "}\n"
    "</style></head><body><kbn-csp data=\"XXXXXX\"></kbn-csp><kbn-injected-metadata data=\"XXXXXX\"></kbn-injected-metadata><style>* {\n"
    "  box-sizing: border-box;\n"
    "}\n"
    "\n"
    "body, html {\n"
    "  width: 100%;\n"
    "  height: 100%;\n"
    "  margin: 0;\n"
    "  background-color: #F5F7FA;\n"
    "}\n"
    ".kibanaWelcomeView {\n"
    "  background-color: #F5F7FA;\n"
    "}\n"
    "\n"
    ".kibanaWelcomeTitle {\n"
    "  color: #000;\n"
    "  font-size: 20px;\n"
    "  font-family: Sans-serif;\n"
    "  margin-top: 20px;\n"
    "  animation: fadeIn 1s ease-in-out;\n"
    "  animation-fill-mode: forwards;\n"
    "  opacity: 0;\n"
    "  animation-delay: 1.0s;\n"
    "}\n"
    "\n"
    ".kibanaWelcomeText {\n"
    "  font-size: 14px;\n"
    "  font-family: Sans-serif;\n"
    "  color: #98A2B3;\n"
    "  animation: fadeIn 1s ease-in-out;\n"
    "  animation-fill-mode: forwards;\n"
    "  opacity: 0;\n"
    "  animation-delay: 1.0s;\n"
    "}\n"
    "\n"
    ".kibanaLoaderWrap {\n"
    "  height: 128px;\n"
    "  width: 128px;\n"
    "  position: relative;\n"
    "  margin-top: 40px;\n"
    "}\n"
    "\n"
    ".kibanaLoaderWrap + * {\n"
    "  margin-top: 24px;\n"
    "}\n"
    "\n"
    ".kibanaLoader {\n"
    "  height: 128px;\n"
    "  width: 128px;\n"
    "  margin: 0 auto;\n"
    "  position: relative;\n"
    "  border: 2px solid transparent;\n"
    "  border-top: 2px solid #017D73;\n"
    "  border-radius: 100%;\n"
    "  display: block;\n"
    "  opacity: 0;\n"
    "  animation: rotation .75s .5s infinite linear, fadeIn 1s .5s ease-in-out forwards;\n"
    "}\n"
    "\n"
    ".kibanaWelcomeLogoCircle {\n"
    "  position: absolute;\n"
    "  left: 4px;\n"
    "  top: 4px;\n"
    "  width: 120px;\n"
    "  height: 120px;\n"
    "  padding: 20px;\n"
    "  background-color: #FFF;\n"
    "  border-radius: 50%;\n"
    "  animation: bounceIn .5s ease-in-out;\n"
    "}\n"
    "\n"
    ".kibanaWelcomeLogo {\n"
    "  background-image: url(\"XXXXXX\");\n"
    "  background-repeat: no-repeat;\n"
    "  background-size: contain;\n"
    "  width: 60px;\n"
    "  height: 60px;\n"
    "  margin: 10px 0px 10px 20px;\n"
    "}\n"
    "\n"
    "@keyframes rotation {\n"
    "  from {\n"
    "    transform: rotate(0deg);\n"
    "  }\n"
    "  to {\n"
    "    transform: rotate(359deg);\n"
    "  }\n"
    "}\n"
    "@keyframes fadeIn {\n"
    "  from {\n"
    "    opacity: 0;\n"
    "  }\n"
    "  to {\n"
    "    opacity: 1;\n"
    "  }\n"
    "}\n"
    "\n"
    "@keyframes bounceIn {\n"
    "  0% {\n"
    "    opacity: 0;\n"
    "    transform: scale(.1);\n"
    "  }\n"
    "  80% {\n"
    "    opacity: .5;\n"
    "    transform: scale(1.2);\n"
    "  }\n"
    "  100% {\n"
    "    opacity: 1;\n"
    "    transform: scale(1);\n"
    "  }\n"
    "}\n"
    "</style><div class=\"XXXXXX\" id=\"XXXXXX\" style=\"XXXXXX\" data-test-subj=\"XXXXXX\"><div class=\"XXXXXX\"><div class=\"XXXXXX\"></div><div class=\"XXXXXX\"><div class=\"XXXXXX\"></div></div></div><div class=\"XXXXXX\" data-error-message=\"XXXXXX\">Loading Kibana</div></div><div class=\"XXXXXX\" id=\"XXXXXX\" style=\"XXXXXX\"><div class=\"XXXXXX\"><div class=\"XXXXXX\"><div class=\"XXXXXX\"></div></div></div><h2 class=\"XXXXXX\">Please upgrade your browser</h2><div class=\"XXXXXX\">This Kibana installation has strict security requirements enabled that your current browser does not meet.</div></div><script>// Since this is an unsafe inline script, this code will not run\n"
    "// in browsers that support content security policy(CSP). This is\n"
    "// intentional as we check for the existence of __kbnCspNotEnforced__ in\n"
    "// bootstrap.\n"
    "window.__kbnCspNotEnforced__ = true;</script><script src=\"XXXXXX\"></script></body></html>";

  std::string cmd = oss.str();
  process_external_command(cmd.c_str());
  checks::checker::instance().reap();
  ASSERT_EQ(_svc->get_state_type(), checkable::hard);
  ASSERT_EQ(_svc->get_current_state(), engine::service::state_ok);
  ASSERT_EQ(_svc->get_last_hard_state_change(), now);
  ASSERT_EQ(_svc->get_perf_data(), "'time'=0.123s;0:3;0:5;0; 'size'=81439B;;;0;");
  ASSERT_EQ(_svc->get_current_attempt(), 1);
  oss.str("");
  retention::dump::services(oss);
  std::string str(oss.str());
  ASSERT_NE(str.find("performance_data='time'=0.123s;0:3;0:5;0; 'size'=81439B;;;0;\n"), std::string::npos);

  std::shared_ptr<comment> cmt = std::make_shared<comment>(comment::service, comment::flapping, _svc->get_host_id(), _svc->get_service_id(), time(nullptr), "test1", "test2", false, comment::internal, false, (time_t)0);

  comment::comments.insert({cmt->get_comment_id(), cmt});

  oss.str("");
  retention::dump::comments(oss);
  ASSERT_NE(str.find("host_name=test_host"), std::string::npos);
  ASSERT_NE(str.find("service_description=test_svc"), std::string::npos);
}
