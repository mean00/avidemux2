<?xml version="1.0" encoding="utf-8" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="utf-8" indent="yes"/>
  <xsl:key name="messageKey" match="source[not(../location[substring(@filename, string-length(@filename) - 2) = '.ui' or contains(@filename, '/qt4/') or contains(@filename, '/ADM_QT4/')]) and not(../translation/@type = 'obsolete')]" use="."/>

  <xsl:template match="TS">
    <TS>
      <xsl:copy-of select="@*"/>
      <xsl:for-each select="context">
        <xsl:call-template name="processContext"/>
      </xsl:for-each>
    </TS>
  </xsl:template>

  <xsl:template name="processContext">
    <context>
      <xsl:choose>
        <xsl:when test="message/location[substring(@filename, string-length(@filename) - 2) = '.ui' or contains(@filename, '/qt4/') or contains(@filename, '/ADM_QT4/')]">
          <xsl:copy-of select="*"/>
        </xsl:when>
        <xsl:otherwise>
          <name/>
          <xsl:for-each select="message/source[generate-id() = generate-id(key('messageKey', .))]">
            <message>
              <xsl:copy-of select="../*"/>
            </message>
          </xsl:for-each>
        </xsl:otherwise>
      </xsl:choose>
    </context>
  </xsl:template>
</xsl:stylesheet>
