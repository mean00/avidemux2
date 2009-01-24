<?xml version="1.0" encoding="utf-8"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="xml" encoding="utf-8" />
	
	<xsl:template match="TS">
		<xsl:text disable-output-escaping="yes"><![CDATA[<!DOCTYPE TS>]]></xsl:text>
		
		<TS>
			<xsl:copy-of select="@*" />

			<xsl:for-each select="context/message">
				<xsl:variable name="location" select="location/@filename" />

				<context>
					<name>
						<xsl:if test ="substring($location, string-length($location) - 2) = '.ui'">
							<xsl:value-of select="../name" />
						</xsl:if>
					</name>

					<message>
						<xsl:copy-of select="*" />
					</message>
				</context>
			</xsl:for-each>
		</TS>
	</xsl:template>
</xsl:stylesheet>