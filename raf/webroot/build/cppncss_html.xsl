<?xml version="1.0" encoding="ISO-8859-1"?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">



    <xsl:template match="/">

      <html>

        <head>

          <title>CppNcss Measurement Results</title>
          <style type="text/css">
table.wiki {
 border: 1px solid #ccc;
 border-collapse: collapse;
 border-spacing: 0;
}
table.wiki td { border: 1px solid #ccc;  padding: .1em .25em; }
table.wiki th {
 border: 1px solid #bbb;
 padding: .1em .25em;
 background-color: #f7f7f7;
}

/* Styles for tabular listings such as those used for displaying directory
   contents and report results. */
table.listing {
 clear: both;
 border-bottom: 1px solid #d7d7d7;
 border-collapse: collapse;
 border-spacing: 0;
 margin-top: 1em;
 width: 100%;
}
table.listing th { text-align: left; padding: 0 1em .1em 0; font-size: 12px }
table.listing thead { background: #f7f7f0 }
table.listing thead th {
 border: 1px solid #d7d7d7;
 border-bottom-color: #999;
 font-size: 11px;
 font-weight: bold;
 padding: 2px .5em;
 vertical-align: bottom;
 white-space: nowrap;
}
table.listing thead th :link:hover, table.listing thead th :visited:hover {
 background-color: transparent;
}
table.listing thead th a { border: none; padding-right: 12px }
table.listing th.asc a, table.listing th.desc a {
 font-weight: bold;
 background-position: 100% 50%;
 background-repeat: no-repeat;
}
table.listing th.asc a { background-image: url(../asc.png) }
table.listing th.desc a { background-image: url(../desc.png) }
table.listing tbody td, table.listing tbody th {
 border: 1px dotted #ddd;
 padding: .3em .5em;
 vertical-align: top;
}
table.listing tbody td a:hover, table.listing tbody th a:hover {
 background-color: transparent;
}
table.listing tbody tr { border-top: 1px solid #ddd }
table.listing tbody tr.even { background-color: #fcfcfc }
table.listing tbody tr.odd { background-color: #f7f7f7 }
table.listing tbody tr:hover { background: #eed !important }
table.listing tbody tr.focus { background: #ddf !important }
          </style>

        </head>

      <body>
        <h1>CppNcss Measurement Results</h1>
        <ul>
        <li><b>NCSS: </b>Non Commenting Source Statements</li>
        <li><b>CCN: </b>Cyclomatic Complexity Number</li>
        </ul>
        <hr/>
        <xsl:apply-templates/>

      </body>

      </html>

    </xsl:template>



    <xsl:template match="/cppncss">



      <h1>Summary</h1>

      <table class="wiki">

        <xsl:for-each select="measure/sum">

        <tr>

          <xsl:call-template name="alternated-row"/>

          <th><xsl:value-of select="@label"/> total</th>

          <td><xsl:value-of select="@value"/></td>

        </tr>

        </xsl:for-each>

      </table>



      <xsl:for-each select="measure">



        <h1><xsl:value-of select="@type"/>s</h1>

        <h2>Top

        <xsl:call-template name='to-lower'>

            <xsl:with-param name='string' select='@type'/>

        </xsl:call-template>s containing the most NCSS</h2>

        <table class="listing">

          <tr class="a">

            <xsl:for-each select="labels/label">

            <th><xsl:value-of select="."/></th>

            </xsl:for-each>

            <th><xsl:value-of select="@type"/></th>

          </tr>

          <xsl:for-each select="item">

          <tr>

            <xsl:call-template name="alternated-row"/>

            <xsl:for-each select="value">

            <td><xsl:value-of select="."/></td>

            </xsl:for-each>

            <td><xsl:value-of select="@name"/></td>

          </tr>

          </xsl:for-each>

        </table>



        <h2>Averages</h2>

        <table class="wiki">

          <xsl:for-each select="average">

          <tr>

            <xsl:call-template name="alternated-row"/>

            <th>Average <xsl:value-of select="@label"/></th>

            <td><xsl:value-of select="@value"/></td>

          </tr>

          </xsl:for-each>

        </table>



      </xsl:for-each>



    </xsl:template>



    <xsl:template name='to-lower'>

        <xsl:param name='string'/>

        <xsl:variable name="lcletters">abcdefghijklmnopqrstuvwxyz</xsl:variable>

        <xsl:variable name="ucletters">ABCDEFGHIJKLMNOPQRSTUVWXYZ</xsl:variable>

        <xsl:value-of select="translate($string,$ucletters,$lcletters)"/>

    </xsl:template>



    <xsl:template name="alternated-row">

      <xsl:attribute name="class">

        <xsl:if test="position() mod 2 = 1">a</xsl:if>

        <xsl:if test="position() mod 2 = 0">b</xsl:if>

      </xsl:attribute>

    </xsl:template>



</xsl:stylesheet>

