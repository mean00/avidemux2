 /***************************************************************************
                             presetOptions.cpp

	These settings are intended for external configuration files and don't
	contain a Preset Configuration block but may contain an Encode Options
	block.

    begin                : Fri May 23 2008
    copyright            : (C) 2008 by gruntster
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <libxml/parser.h>
#include "ADM_default.h"
#include "presetOptions.h"

x264PresetOptions::x264PresetOptions(void) : x264Options()
{
	setEncodeOptionsToDefaults();
}

vidEncOptions* x264PresetOptions::getEncodeOptions(void)
{
	vidEncOptions *encodeOptions = new vidEncOptions;

	memcpy(encodeOptions, &_encodeOptions, sizeof(vidEncOptions));

	return encodeOptions;
}

void x264PresetOptions::setEncodeOptions(vidEncOptions* encodeOptions)
{
	memcpy(&_encodeOptions, encodeOptions, sizeof(vidEncOptions));
}

char* x264PresetOptions::toXml(void)
{
	xmlDocPtr xmlDoc = xmlNewDoc((const xmlChar*)"1.0");
	xmlNodePtr xmlNodeRoot, xmlNodeChild;
	const int bufferSize = 100;
	xmlChar xmlBuffer[bufferSize + 1];
	char *xml = NULL;

	while (true)
	{
		if (!xmlDoc)
			break;

		if (!(xmlNodeRoot = xmlNewDocNode(xmlDoc, NULL, (xmlChar*)"x264Config", NULL)))
			break;

		xmlDocSetRootElement(xmlDoc, xmlNodeRoot);

		xmlNodeChild = xmlNewChild(xmlNodeRoot, NULL, (xmlChar*)"encodeOptions", NULL);

		switch (_encodeOptions.encodeMode)
		{
			case ADM_VIDENC_MODE_CBR:
				strcpy((char*)xmlBuffer, "CBR");
				break;
			case ADM_VIDENC_MODE_CQP:
				strcpy((char*)xmlBuffer, "CQP");
				break;
			case ADM_VIDENC_MODE_AQP:
				strcpy((char*)xmlBuffer, "AQP");
				break;
			case ADM_VIDENC_MODE_2PASS_SIZE:
				strcpy((char*)xmlBuffer, "2PASS SIZE");
				break;
			case ADM_VIDENC_MODE_2PASS_ABR:
				strcpy((char*)xmlBuffer, "2PASS ABR");
				break;
		}

		xmlNewChild(xmlNodeChild, NULL, (xmlChar*)"mode", xmlBuffer);
		xmlNewChild(xmlNodeChild, NULL, (xmlChar*)"parameter", number2String(xmlBuffer, bufferSize, _encodeOptions.encodeModeParameter));

		addX264OptionsToXml(xmlNodeRoot);
		xml = dumpXmlDocToMemory(xmlDoc);
		xmlFreeDoc(xmlDoc);

		break;
	}

	return xml;
}

int x264PresetOptions::fromXml(const char *xml)
{
	bool success = false;

	clearPresetConfiguration();
	clearZones();
	setEncodeOptionsToDefaults();

	xmlDocPtr doc = xmlReadMemory(xml, strlen(xml), "x264.xml", NULL, 0);

	if (success = validateXml(doc))
	{
		xmlNode *xmlNodeRoot = xmlDocGetRootElement(doc);

		for (xmlNode *xmlChild = xmlNodeRoot->children; xmlChild; xmlChild = xmlChild->next)
		{
			if (xmlChild->type == XML_ELEMENT_NODE)
			{
				char *content = (char*)xmlNodeGetContent(xmlChild);

				if (strcmp((char*)xmlChild->name, "encodeOptions") == 0)
					parseEncodeOptions(xmlChild, &_encodeOptions);
				else if (strcmp((char*)xmlChild->name, "x264Options") == 0)
					parseX264Options(xmlChild);

				xmlFree(content);
			}
		}
	}

	xmlFreeDoc(doc);

	return success;
}

void x264PresetOptions::parseEncodeOptions(xmlNode *node, vidEncOptions *encodeOptions)
{
	for (xmlNode *xmlChild = node->children; xmlChild; xmlChild = xmlChild->next)
	{
		if (xmlChild->type == XML_ELEMENT_NODE)
		{
			char *content = (char*)xmlNodeGetContent(xmlChild);

			if (strcmp((char*)xmlChild->name, "mode") == 0)
			{
				if (strcmp(content, "CBR") == 0)
					encodeOptions->encodeMode = ADM_VIDENC_MODE_CBR;
				else if (strcmp(content, "CQP") == 0)
					encodeOptions->encodeMode = ADM_VIDENC_MODE_CQP;
				else if (strcmp(content, "AQP") == 0)
					encodeOptions->encodeMode = ADM_VIDENC_MODE_AQP;
				else if (strcmp(content, "2PASS SIZE") == 0)
					encodeOptions->encodeMode = ADM_VIDENC_MODE_2PASS_SIZE;
				else if (strcmp(content, "2PASS ABR") == 0)
					encodeOptions->encodeMode = ADM_VIDENC_MODE_2PASS_ABR;
			}
			else if (strcmp((char*)xmlChild->name, "parameter") == 0)
				encodeOptions->encodeModeParameter = atoi(content);

			xmlFree(content);
		}
	}
}

void x264PresetOptions::setEncodeOptionsToDefaults(void)
{
	_encodeOptions.encodeMode = DEFAULT_ENCODE_MODE;
	_encodeOptions.encodeModeParameter = DEFAULT_ENCODE_MODE_PARAMETER;
}
