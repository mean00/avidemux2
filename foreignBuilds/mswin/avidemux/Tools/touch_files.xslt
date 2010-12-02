<xsl:stylesheet version="1.0"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="html" indent="yes" />

  <xsl:template match="/">
    <xsl:text disable-output-escaping="yes">&lt;?php
      function touchDirContents($path, $time)
      {
        $dirHandle = opendir($path);

        while (($resource = readdir($dirHandle)) != false)
        {
          if (is_file($path . "/" . $resource))
          {
            touch($path . "/" . $resource, $time);
          }
        }

        closedir($dirHandle);
      }
      </xsl:text>
    <xsl:apply-templates select="log" />
    <xsl:text disable-output-escaping="yes">?&gt;</xsl:text>
  </xsl:template>

  <xsl:template match="log" mode="php">
    <script language="php">
      <xsl:apply-templates select="buildentry" />
    </script>
  </xsl:template>

  <xsl:template match="buildentry">
    <xsl:variable name="fileName">
      <xsl:call-template name="getFileName">
        <xsl:with-param name="revision" select="@revision" />
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="day">
      <xsl:call-template name="getDay">
        <xsl:with-param name="date" select="@date" />
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="month">
      <xsl:call-template name="getMonth">
        <xsl:with-param name="date" select="@date" />
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="year">
      <xsl:call-template name="getYear">
        <xsl:with-param name="date" select="@date" />
      </xsl:call-template>
    </xsl:variable>

    <xsl:text>touch("</xsl:text>
    <xsl:value-of select="$fileName" />
    <xsl:text>", mktime(0, 0, 0, </xsl:text>
    <xsl:value-of select="$month" />
    <xsl:text>, </xsl:text>
    <xsl:value-of select="$day" />
    <xsl:text>, </xsl:text>
    <xsl:value-of select="$year" />
    <xsl:text>)); </xsl:text>

    <xsl:text>touchDirContents("</xsl:text>
    <xsl:value-of select="$fileName" />
    <xsl:text>", mktime(0, 0, 0, </xsl:text>
    <xsl:value-of select="$month" />
    <xsl:text>, </xsl:text>
    <xsl:value-of select="$day" />
    <xsl:text>, </xsl:text>
    <xsl:value-of select="$year" />
    <xsl:text>)); </xsl:text>
  </xsl:template>

  <xsl:template name='getDay'>
    <xsl:param name='date' />
    <xsl:variable name="day" select="substring(@date, 9, 2)"/>
    <xsl:choose>
      <xsl:when test="starts-with($day, '0')">
        <xsl:value-of select="substring($day, 2)" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$day" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name='getMonth'>
    <xsl:param name='date' />
    <xsl:variable name="month" select="substring(@date, 6, 2)"/>
    <xsl:choose>
      <xsl:when test="starts-with($month, '0')">
        <xsl:value-of select="substring($month, 2)" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$month" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name='getYear'>
    <xsl:param name='date' />
    <xsl:value-of select="substring(@date, 1, 4)"/>
  </xsl:template>

  <xsl:template name='getFileName'>
    <xsl:param name='revision' />

    <xsl:text>2.5/</xsl:text>

    <xsl:choose>
      <xsl:when test="contains($revision, 'Final')">
        <xsl:text>Milestone/</xsl:text>
        <xsl:variable name="version" select="substring-after($revision, ' [')"/>
        <xsl:value-of select="substring-before($version, ' ')" />
        <xsl:text>_(</xsl:text>
        <xsl:value-of select="substring-before($revision, ' [')" />
        <xsl:text>)</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>SVN/</xsl:text>
        <xsl:choose>
          <xsl:when test="contains($revision, 'Preview')">
            <xsl:variable name="version" select="substring-after($revision, ' [')"/>
            <xsl:variable name="minorVersion" select="substring-after($version, ' ')"/>
            <xsl:value-of select="substring-before($revision, ' [')"/>
            <xsl:text>_</xsl:text>
            <xsl:call-template name="convertToLower">
              <xsl:with-param name="toconvert" select="translate(substring-before($minorVersion, ']'), ' ', '_')" />
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="contains($revision, 'Release')">
            <xsl:value-of select="substring-before($revision, ' [')" />
            <xsl:text>_r</xsl:text>
            <xsl:variable name="version" select="substring-after($revision, ' [Release ')"/>
            <xsl:value-of select="substring-before($version, ']')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$revision"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name='convertToLower'>
    <xsl:param name='toconvert' />
    <xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>
    <xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>
    <xsl:value-of select="translate($toconvert,$ucletters,$lcletters)" />
  </xsl:template>
</xsl:stylesheet>