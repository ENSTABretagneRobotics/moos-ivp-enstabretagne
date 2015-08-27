/****************************************************************/
/*    FILE: MOOSAppDoc.cpp
/*    ORGN: Toutatis AUVs - ENSTA Bretagne
/*    AUTH: Simon Rohou
/*    DATE: 2015
/****************************************************************/

#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "ColorParse.h"
#include "MOOSAppDocumentation.h"

using namespace std;
using namespace tinyxml2;

namespace xmldoc
{
  //---------------------------------------------------------
  // Constructor

  MOOSAppDocumentation::MOOSAppDocumentation(string moosapp_name)
  {
    m_xml_doc = NULL;
    m_moosapp_name = moosapp_name;
  }

  //---------------------------------------------------------
  // Destructor

  MOOSAppDocumentation::~MOOSAppDocumentation()
  {
    delete m_xml_doc;
  }

  //----------------------------------------------------------------
  // Procedure: showSynopsis

  void MOOSAppDocumentation::showSynopsis()
  {
    if(m_xml_doc == NULL) // loaded if necessary
      loadXML();

    blk("SYNOPSIS:                                                       ");
    blk("------------------------------------                            ");
    blk(multiLineMessage(m_synopsis, MESSAGE_LENGTH, "  "));
  }

  //----------------------------------------------------------------
  // Procedure: getRepositoryPath

  string MOOSAppDocumentation::getRepositoryPath()
  {
    // Test if environment value is set
    char* repository_path = getenv(MOOS_IVP_TOUTATIS_PATH);
    if(repository_path == NULL)
    {
      red("  ERROR: unable to read " + string(MOOS_IVP_TOUTATIS_PATH) + " environment value.");
      blk("  Please update your configuration.");
      blk("  For Linux users: add an EXPORT in your ~/.bashrc file. Ex:");
      blk("    export MOOS_IVP_PROJECTNAME_PATH=\"/home/user/moos-ivp-projectname\"\n");
      exit(0);
    }

    // Test if MOOS_IVP_TOUTATIS_PATH is an absolute path
    if(repository_path[0] != '/')
    {
      red("  ERROR: " + string(MOOS_IVP_TOUTATIS_PATH) + " is not an absolute path.");
      blk("  Please update your configuration.");
      blk("  For Linux users: check the EXPORT in your ~/.bashrc file. Ex:");
      blk("    export MOOS_IVP_PROJECTNAME_PATH=\"/home/user/moos-ivp-projectname\"\n");
      exit(0);
    }

    return string(repository_path);
  }

  //----------------------------------------------------------------
  // Procedure: showSynopsis

  void MOOSAppDocumentation::loadXML()
  {
    string repository_path = getRepositoryPath();
    string doc_path = repository_path + "/src/app/" + m_moosapp_name + "/" + m_moosapp_name + ".xml";

    // Test file existance with ifstream
    ifstream xmlfile(doc_path.c_str());
    if(!xmlfile)
    {
      red("  ERROR: unable to load " + m_moosapp_name + ".xml documentation file.");
      blk("  Please check file existance at:");
      blk("  " + doc_path + "\n");
      exit(0);
    }

    // Test XML global validity
    m_xml_doc = new XMLDocument();
    if(m_xml_doc->LoadFile(doc_path.c_str()) != XML_NO_ERROR)
    {
      red("  ERROR: unable to load " + m_moosapp_name + ".xml documentation file.");
      blk("  Please check XML validity at:");
      blk("  " + doc_path + "\n");
      exit(0);
    }

    // Test XML items validity
    string item_error = "";
    if(!parseXML(item_error))
    {
      red("  ERROR: parsing " + m_moosapp_name + ".xml documentation file.");
      if(item_error != "")
        blk("  Unable to read <" + item_error + ">");
      blk("  Please check XML validity at:");
      blk("  " + doc_path + "\n");
      exit(0);
    }
  }

  //----------------------------------------------------------------
  // Procedure: parseXML

  bool MOOSAppDocumentation::parseXML(string &item_error)
  {
    // Parsing...
    XMLElement *element;
    XMLNode *root = m_xml_doc->FirstChildElement("moosapp");
    if(root == NULL) { item_error = "moosapp"; return false; }

    // Info section
    {
      XMLElement *info_section = root->FirstChildElement("info");
      if(info_section == NULL) { item_error = "info"; return false; }

      // Organization
      element = info_section->FirstChildElement("organization");
      if(element == NULL) { item_error = "organization"; return false; }
      m_info_organization = element->GetText();
      MOOSTrimWhiteSpace(m_info_organization);

      // Date
      element = info_section->FirstChildElement("date");
      if(element == NULL) { item_error = "date"; return false; }
      m_info_date = element->GetText();
      MOOSTrimWhiteSpace(m_info_date);

      // Licence
      element = info_section->FirstChildElement("licence");
      if(element == NULL) { item_error = "licence"; return false; }
      m_info_licence = element->GetText();
      MOOSTrimWhiteSpace(m_info_licence);

      // Authors
      element = info_section->FirstChildElement("authors");
      if(element == NULL) { item_error = "authors"; return false; }
      XMLElement *author = element->FirstChildElement("author");
      while(author != NULL)
      {
        string author_name;
        author_name = author->GetText();
        MOOSTrimWhiteSpace(author_name);
        m_info_authors.push_back(author_name);
        author = author->NextSiblingElement("author");
      }
    }

    // Documentation section
    {
      XMLElement *documentation_section = root->FirstChildElement("documentation");
      if(documentation_section == NULL) { item_error = "documentation"; return false; }

      // Synopsis
      element = documentation_section->FirstChildElement("synopsis");
      if(element == NULL) { item_error = "synopsis"; return false; }
      m_synopsis = element->GetText();
      MOOSTrimWhiteSpace(m_synopsis);

      // Optional comments
      element = documentation_section->FirstChildElement("optional-comments");
      if(element == NULL) { item_error = "optional-comments"; return false; }
      m_optional_comments = element->GetText();
      MOOSTrimWhiteSpace(m_optional_comments);

      // Suggested improvements
      element = documentation_section->FirstChildElement("suggested-improvements");
      if(element == NULL) { item_error = "suggested-improvements"; return false; }
      m_suggested_improvements = element->GetText();
      MOOSTrimWhiteSpace(m_suggested_improvements);

      // Interface
      {
        XMLElement *interface_subsection = documentation_section->FirstChildElement("interface");
        if(interface_subsection == NULL) { item_error = "interface"; return false; }

        // Subscriptions
        XMLElement *subscriptions_subsection = interface_subsection->FirstChildElement("subscriptions");
        if(subscriptions_subsection == NULL) { item_error = "subscriptions"; return false; }
        XMLElement *moosvar_xml = subscriptions_subsection->FirstChildElement("moosvar");
        while(moosvar_xml != NULL)
        {
          MOOSVarDescription moosvar;
          if(!xmlToMoosvar(moosvar_xml, moosvar, item_error)) { return false; }
          m_subscriptions.push_back(moosvar);
          moosvar_xml = moosvar_xml->NextSiblingElement("moosvar");
        }

        // Publications
        XMLElement *publications_subsection = interface_subsection->FirstChildElement("publications");
        if(publications_subsection == NULL) { item_error = "publications"; return false; }
        moosvar_xml = publications_subsection->FirstChildElement("moosvar");
        while(moosvar_xml != NULL)
        {
          MOOSVarDescription moosvar;
          if(!xmlToMoosvar(moosvar_xml, moosvar, item_error)) { return false; }
          m_publications.push_back(moosvar);
          moosvar_xml = moosvar_xml->NextSiblingElement("moosvar");
        }
      }
    }

    return true;
  }

  //----------------------------------------------------------------
  // Procedure: parseXML

  bool MOOSAppDocumentation::xmlToMoosvar(XMLElement *xmlmoosvar, MOOSVarDescription &moosvar, string &item_error)
  {
    string value;
    XMLElement *element;

    // Name
    element = xmlmoosvar->FirstChildElement("name");
    if(element == NULL) { item_error = "moosvar/name"; return false; }
    value = element->GetText();
    MOOSTrimWhiteSpace(value);
    moosvar.setName(value);

    // Type
    element = xmlmoosvar->FirstChildElement("type");
    if(element == NULL) { item_error = "moosvar/type"; return false; }
    value = element->GetText();
    MOOSTrimWhiteSpace(value);
    moosvar.setType(value);

    // Info
    element = xmlmoosvar->FirstChildElement("info");
    if(element == NULL) { item_error = "moosvar/info"; return false; }
    value = element->GetText();
    MOOSTrimWhiteSpace(value);
    moosvar.setInfo(value);

    return true;
  }

  //----------------------------------------------------------------
  // Procedure: showTitleSection

  void MOOSAppDocumentation::showTitleSection(string title_section)
  {
    blk("                                                                ");
    blu("================================================================");
    blu(("  " + m_moosapp_name + " - " + title_section).c_str());
    blu("================================================================");
    blk("                                                                ");
  }

  //----------------------------------------------------------------
  // Procedure: showHelpAndExit

  void MOOSAppDocumentation::showHelpAndExit()
  {
    showTitleSection("HELP");
    showSynopsis();

    blk("                                                                ");
    blk("Options:                                                        ");
    mag("  --alias","=<ProcessName>                                      ");
    blk("      Launch " + m_moosapp_name + " with the given process name ");
    blk("      rather than " + m_moosapp_name + ".                       ");
    mag("  --example, -e                                                 ");
    blk("      Display example MOOS configuration block.                 ");
    mag("  --help, -h                                                    ");
    blk("      Display this help message.                                ");
    mag("  --interface, -i                                               ");
    blk("      Display MOOS publications and subscriptions.              ");
    mag("  --version,-v                                                  ");
    blk("      All information about " + m_moosapp_name + ".             ");
    blk("                                                                ");
    blk("Note: If argv[2] does not otherwise match a known option,       ");
    blk("      then it will be interpreted as a run alias. This is       ");
    blk("      to support pAntler launching conventions.                 ");
    blk("                                                                ");

    exit(0);
  }

  //----------------------------------------------------------------
  // Procedure: showExampleConfigAndExit

  void MOOSAppDocumentation::showExampleConfigAndExit()
  {
    showTitleSection("EXAMPLE MOOS CONFIGURATION");

    string repository_path = getRepositoryPath();
    string example_path = repository_path + "/src/app/" + m_moosapp_name + "/" + m_moosapp_name + ".moos";

    // Test file existance with ifstream
    ifstream moosfile(example_path.c_str());
    if(!moosfile || !moosfile.is_open())
    {
      red("  ERROR: unable to load " + m_moosapp_name + ".moos example file.");
      blk("  Please check file existance at:");
      blk("  " + example_path + "\n");
      exit(0);
    }

    // Write .moos configuration file
    string line;
    while(getline(moosfile, line))
      blk(line);

    blk("");
    moosfile.close();
    exit(0);
  }

  //----------------------------------------------------------------
  // Procedure: showInterfaceAndExit

  void MOOSAppDocumentation::showInterfaceAndExit()
  {
    showTitleSection("INTERFACE");

    if(m_xml_doc == NULL) // loaded if necessary
      loadXML();

    blk("SUBSCRIPTIONS:                                                  ");
    blk("------------------------------------                            ");

    if(m_subscriptions.empty())
      blk("  No subscription.                                            ");

    else
      for(int i = 0 ; i < m_subscriptions.size() ; i++)
        blk(m_subscriptions[i].toString());

    blk("                                                                ");
    blk("PUBLICATIONS:                                                   ");
    blk("------------------------------------                            ");

    if(m_publications.empty())
      blk("  No publication.                                             ");

    else
      for(int i = 0 ; i < m_publications.size() ; i++)
        blk(m_publications[i].toString());

    blk("                                                                ");

    exit(0);
  }

  //----------------------------------------------------------------
  // Procedure: showReleaseInfoAndExit

  void MOOSAppDocumentation::showReleaseInfoAndExit()
  {
    showTitleSection("RELEASE INFO");

    if(m_xml_doc == NULL) // loaded if necessary
      loadXML();

    showSynopsis();

    blk("                                                                ");
    blk("AUTHORS:                                                        ");
    blk("------------------------------------                            ");
    for(int i = 0 ; i < m_info_authors.size() ; i++)
      blk(multiLineMessage("- " + m_info_authors[i], MESSAGE_LENGTH, "  "));
    blk("                                                                ");

    blk("CONTEXT:                                                        ");
    blk("------------------------------------                            ");
    blk("  Date: " + m_info_date);
    blk("  Organization: " + m_info_organization);
    blk("  Licence: " + m_info_licence);
    blk("                                                                ");

    blk("OPTIONAL COMMENTS:                                              ");
    blk("------------------------------------                            ");
    blk(multiLineMessage(m_optional_comments, MESSAGE_LENGTH, "  "));
    blk("                                                                ");

    blk("SUGGESTED IMPROVEMENTS:                                         ");
    blk("------------------------------------                            ");
    blk(multiLineMessage(m_suggested_improvements, MESSAGE_LENGTH, "  "));
    blk("                                                                ");

    exit(0);
  }

  //----------------------------------------------------------------
  // Procedure: multiLineMessage

  string multiLineMessage(string message, int line_length, string indent)
  {
    int j = 0;
    string multiline_message = indent;

    for(int i = 0 ; i < message.length() ; i++)
    {
      j ++;

      multiline_message += message[i];
      if(message[i] == ' ' && j > line_length)
      {
        j = 0;
        multiline_message += '\n' + indent;
      }
    }

    return multiline_message;
  }
}