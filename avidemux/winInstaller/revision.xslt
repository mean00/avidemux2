<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text"/>

  <xsl:template match="/">
    <xsl:text>!define REVISION </xsl:text>
    <xsl:value-of select="log/logentry/@revision"/>
  </xsl:template>
</xsl:stylesheet>