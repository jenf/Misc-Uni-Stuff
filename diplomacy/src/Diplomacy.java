// Diplomacy.java
// Most code is work of Jennifer Freeman, 2001
/* Some of this code is based on an example from Apache's crimson-1.1.3 xml parser 
   The startElement, endElement, characters, endDocument, startDocument, main have been
   rewritten to load the data for diplomacy */
 
import javax.xml.parsers.*;
import org.xml.sax.*;
import org.xml.sax.helpers.*;
import javax.swing.*;
import java.util.*;
import java.io.*;

public class Diplomacy extends DefaultHandler {

    private String current;
    private String neighbourname;
    private String neighbourfrom;
    private String neighbourto;
    private String currentcountry;
    
    // This gets called when an element starts
    public void startElement(String namespaceURI, String localName,
                             String qName, Attributes attrs)
	throws SAXException
    {
        if (qName.equals("province"))
        {
         Country n;
         String bar;
         String pa,pb;

         boolean foo=false;
         bar=attrs.getValue("supply");
         if (bar==null) foo=false;
         else {if (bar.equals("yes")) foo=true;}
         pa=attrs.getValue("coastA");
         pb=attrs.getValue("coastB");
         currentcountry=attrs.getValue("short");
         try
         {
          Map.addCountry(currentcountry,pa,pb,attrs.getValue("name"),
                         attrs.getValue("type"),foo);
         }
         catch (java.lang.Exception exception)
         {
        	 System.out.println(exception.toString());
         }
        }
        if (qName.equals("neighbour"))
        {
         neighbourname=new String();
         neighbourfrom=attrs.getValue("from");
         neighbourto=attrs.getValue("to");
        }
        current=qName;
    }
    // This gets called when we get a bit of data inside tags.
    public void characters(char[] ch, int start, int length)
    { 
        if (current.equals("neighbour"))
        {
            String bar;
            bar=new String(ch,start,length);
            neighbourname+=bar;
        }
    }
    
    // This gets called when an element has ended.
    public void endElement(String namespaceURI, String localName,
                             String qName) throws SAXException
    {
        if (qName.equals("neighbour"))
        {
           Map.AddLink(currentcountry,neighbourname,neighbourfrom,neighbourto);
        }
    }
    
    // this gets called when we've got to the end of the file.
    public void endDocument() throws SAXException
    {
        Country bar;
        bar=Map.getCountry("ber");
        // Check the file is complete
        System.out.println("--Running Consistancy Checker--");
        bar.check();
        System.out.println("--Complete--");
    }

    /**
     * Convert from a filename to a file URL.
     */
    // Part of the crimson example
    private static String convertToFileURL(String filename) {
        // On JDK 1.2 and later, simplify this to:
        // "path = file.toURL().toString()".
        String path = new File(filename).getAbsolutePath();
        if (File.separatorChar != '/') {
            path = path.replace(File.separatorChar, '/');
        }
        if (!path.startsWith("/")) {
            path = "/" + path;
        }
        return "file:" + path;
    }
    
    static public void main(String[] args) {
        // Create a JAXP SAXParserFactory and configure it
        SAXParserFactory spf = SAXParserFactory.newInstance();
        // we don't want to check it against the dtd.
        spf.setValidating(false);

        XMLReader xmlReader = null;
        try {
            // Create a JAXP SAXParser
            SAXParser saxParser = spf.newSAXParser();

            // Get the encapsulated SAX XMLReader
            xmlReader = saxParser.getXMLReader();
        } catch (Exception ex) {
            System.err.println(ex);
            System.exit(1);
        }

        // Set the ContentHandler of the XMLReader
        xmlReader.setContentHandler(new Diplomacy());

        // Set an ErrorHandler before parsing
        xmlReader.setErrorHandler(new MyErrorHandler(System.err));

        try {
            // Tell the XMLReader to parse the XML document
            // The following three lines are mine
            xmlReader.parse(convertToFileURL("./diplomacy.xml"));
            JFrame frame=new DipGui();
            frame.setVisible(true);
        } catch (SAXException se) {
            System.err.println(se.getMessage());
            System.exit(1);
        } catch (IOException ioe) {
            System.err.println(ioe);
            System.exit(1);
        }
    }

    // This class is part of the crimson example
    // Error handler to report errors and warnings
    private static class MyErrorHandler implements ErrorHandler {
        /** Error handler output goes here */
        private PrintStream out;

        MyErrorHandler(PrintStream out) {
            this.out = out;
        }

        /**
         * Returns a string describing parse exception details
         */
        private String getParseExceptionInfo(SAXParseException spe) {
            String systemId = spe.getSystemId();
            if (systemId == null) {
                systemId = "null";
            }
            String info = "URI=" + systemId +
                " Line=" + spe.getLineNumber() +
                ": " + spe.getMessage();
            return info;
        }

        // The following methods are standard SAX ErrorHandler methods.
        // See SAX documentation for more info.

        public void warning(SAXParseException spe) throws SAXException {
            out.println("Warning: " + getParseExceptionInfo(spe));
        }
        
        public void error(SAXParseException spe) throws SAXException {
            String message = "Error: " + getParseExceptionInfo(spe);
            throw new SAXException(message);
        }

        public void fatalError(SAXParseException spe) throws SAXException {
            String message = "Fatal Error: " + getParseExceptionInfo(spe);
            throw new SAXException(message);
        }
    }
}
