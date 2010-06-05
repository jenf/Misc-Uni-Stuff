// Map.java
// All code is work of Jennifer Freeman, 2001
import java.util.*;
public abstract class Map
{
   private static TreeMap<String, Country> map=new TreeMap<String, Country>();
   
   public static void addCountry(String shortname,String pa,String pb,
                                  String longname,String type,boolean supply)
   {
     Country n=new Country(longname, pa,pb,new LandType(type), supply);
     map.put(shortname.toLowerCase(),n);
   }
   public static Country getCountry(String shortname)
   {
     return map.get(shortname.toLowerCase());
   }
   public static Object[] getValues()
   {
     return map.keySet().toArray();
   }
   // Java doesn't like static toStrings 
   public static String show()
   {
     return map.toString();
   }
   public static void AddLink(String from, String to,String portfrom,String portto)
   {
     (Map.getCountry(from)).addNeighbour(new Move(to.toLowerCase(),portfrom,portto));
   }
}

