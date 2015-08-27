/****************************************************************/
/*    FILE: MOOSAppDoc.h
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/****************************************************************/

#ifndef MOOSAPPDOC_INFO_HEADER
#define MOOSAPPDOC_INFO_HEADER

#include <string>
#include <vector>
#include <tinyxml2.h>
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"

#define MOOS_IVP_TOUTATIS_PATH "MOOS_IVP_TOUTATIS_PATH"
#define MESSAGE_LENGTH 60

namespace xmldoc
{
  // Utility
  std::string multiLineMessage(std::string message, int line_length = MESSAGE_LENGTH, std::string indent = "");

  class MOOSVarDescription
  {
    public:
      MOOSVarDescription() { m_name = ""; };
      ~MOOSVarDescription() {};

      void setName(std::string name) { MOOSTrimWhiteSpace(name);  m_name = name; };
      void setType(std::string type) { MOOSTrimWhiteSpace(type);  m_type = type; };
      void setInfo(std::string info) { MOOSTrimWhiteSpace(info);  m_info = info; };
      std::string toString() { return multiLineMessage(m_name + " (" + m_type + "): " + m_info, MESSAGE_LENGTH, "  "); };

    private:
      std::string m_name;
      std::string m_type;
      std::string m_info;
  };

  class MOOSAppDocumentation
  {
    public:
      MOOSAppDocumentation(std::string moosapp_name);
      ~MOOSAppDocumentation();

      void showSynopsis();
      void showHelpAndExit();
      void showExampleConfigAndExit();
      void showInterfaceAndExit();
      void showReleaseInfoAndExit();
      void showTitleSection(std::string title_section);

    protected:
      std::string getRepositoryPath();
      void loadXML();
      bool parseXML(std::string &item_error);
      bool xmlToMoosvar(tinyxml2::XMLElement *xmlmoosvar, MOOSVarDescription &moosvar, std::string &item_error);

    private:
      tinyxml2::XMLDocument *m_xml_doc;
      std::string m_moosapp_name;
      std::string m_info_organization;
      std::string m_info_date;
      std::vector<std::string> m_info_authors;
      std::string m_info_licence;
      std::string m_synopsis;
      std::string m_optional_comments;
      std::string m_suggested_improvements;
      std::vector<MOOSVarDescription> m_subscriptions;
      std::vector<MOOSVarDescription> m_publications;
  };
}

#endif