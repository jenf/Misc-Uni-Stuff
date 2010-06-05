// LandType.java
// All code is work of Jennifer Freeman, 2001
public class LandType
{
    private int countryType;
    public LandType (String b)
    { 
      String t=b.toLowerCase();
      if (t.equals("land"))
      {
        countryType=1;
      } else
      {
       if (t.equals("coast"))
       {
         countryType=2;
       } else
       {
        if (t.equals("sea"))
        {
          countryType=3;
        }
       }
      }
    }
    public boolean isLand()
    {
        return countryType==1;
    }
    public boolean isCoast()
    {
        return countryType==2;
    }
    public boolean isSea()
    {
        return countryType==3;
    }
    public String toString()
    {
       if (this.isLand()) return "Land";
       if (this.isCoast()) return "Coast";
       if (this.isSea()) return "Sea";
       return "";
    }
}
