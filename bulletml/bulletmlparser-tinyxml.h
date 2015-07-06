/// BulletML �̃p�[�T�E�c���[ by tinyxml
/**
 * tinyxml�ɂ��BulletML�̃p�[�T
 */

#ifndef BULLETMLPARSER_TINYXML_H_
#define BULLETMLPARSER_TINYXML_H_

#include <string>


#include "tinyxml/tinyxml.h"

class BulletMLNode;

class BulletMLParserTinyXML : public BulletMLParser {
public:
     BulletMLParserTinyXML(const std::string& filename);
     virtual ~BulletMLParserTinyXML();

     virtual void parse();

protected:
	void parseImpl(TiXmlDocument& doc);

protected:
    void getTree(TiXmlNode* node);
    void translateNode(TiXmlNode* node);

private:
    std::string xmlFile_;
    BulletMLNode* curNode_;
};

#endif // ! BULLETMLPARSER_TINYXML_H_
