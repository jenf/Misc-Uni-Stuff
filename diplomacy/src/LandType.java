// LandType.java
// All code is work of Jennifer Freeman, 2001
public enum LandType
{
	Land,Coast,Sea;
    static public LandType parse(String landtype) throws Exception
    {
        String t=landtype.toLowerCase();
        if (t.equals("land"))
        {
          return Land;
        }
        else
        {
         if (t.equals("coast"))
         {
           return Coast;
         }
         else
         {
          if (t.equals("sea"))
          {
            return Sea;
          }
         }
        }	
        throw new Exception("Bad landtype "+landtype);
    }
}

