<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html" indent="yes" />

  <xsl:template match="log">
    <html>
      <head>
        <meta content="text/html; charset=ISO-8859-1" http-equiv="content-type" />
        <style type="text/css">
          body {
          margin: 10px 15px 0px 15px;
          height: 100%;
          font-size: 82%;
          }

          h1 {
          padding: 0;
          font-family: Helvetica,Arial,sans-serif;
          font-size: 1.7em;
          font-weight: bold;
          color: #1B57B1;
          vertical-align: bottom;
          text-align: center;
          width: 100%;
          }

          div, p, table, td, th {
          font-family: Tahoma,Helvetica,Arial,sans-serif;
          font-size: 1em;
          color: #333;
          }

          table.build {
          border-collapse: collapse;
          border: 1px solid #ccc;
          }

          table.build td, table.roadmap th {
          border: 1px solid #ccc;
          }

          table.build th {
          background: #ddd;
          padding: 5px;
          }

          table.build td {
          background: #f7f7f7;
          padding-left: 15px;
          padding-right: 15px;
          padding-top: 5px;
          padding-bottom: 5px;
          }
        </style>
        <title>Avidemux 2.5 Win32 Package Notes</title>
      </head>
      <body>
        <h1>Avidemux 2.5 Win32 Package Notes</h1>
        <div>
          <xsl:apply-templates select="buildentry" />
        </div>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="buildentry">
    <p>
      <b>
        <xsl:text>Revision&#160;</xsl:text>
        <xsl:value-of select="@revision"/>
      </b>
      <xsl:text>&#160;(</xsl:text>
      <xsl:call-template name="FormatDate">
        <xsl:with-param name="Date" select="@date" />
      </xsl:call-template>
      <xsl:text>)</xsl:text>
      <br/>

      <xsl:for-each select="comment">
        <li>
          <xsl:value-of select="."/>
        </li>
      </xsl:for-each>
    </p>
  </xsl:template>

  <xsl:template name="FormatDate">
    <xsl:param name="Date"/>

    <!-- Day -->
    <xsl:value-of select="substring($Date, 9, 2)" />
    <xsl:text>&#160;</xsl:text>

    <!-- Month -->
    <xsl:call-template name="FormatMonth">
      <xsl:with-param name="Month" select="substring($Date, 6, 2)" />
    </xsl:call-template>
    <xsl:text>&#160;</xsl:text>

    <!-- Year -->
    <xsl:value-of select="substring($Date, 1, 4)"/>
    <!-- <xsl:text>&#160;</xsl:text>-->

    <!-- Time -->
    <!-- <xsl:value-of select="substring($Date, 12, 8)"/>-->
  </xsl:template>

  <xsl:template name="FormatMonth">
    <xsl:param name="Month"/>
    <xsl:choose>
      <xsl:when test="$Month = '01'">Jan</xsl:when>
      <xsl:when test="$Month = '02'">Feb</xsl:when>
      <xsl:when test="$Month = '03'">Mar</xsl:when>
      <xsl:when test="$Month = '04'">Apr</xsl:when>
      <xsl:when test="$Month = '05'">May</xsl:when>
      <xsl:when test="$Month = '06'">Jun</xsl:when>
      <xsl:when test="$Month = '07'">Jul</xsl:when>
      <xsl:when test="$Month = '08'">Aug</xsl:when>
      <xsl:when test="$Month = '09'">Sep</xsl:when>
      <xsl:when test="$Month = '10'">Oct</xsl:when>
      <xsl:when test="$Month = '11'">Nov</xsl:when>
      <xsl:when test="$Month = '12'">Dec</xsl:when>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>