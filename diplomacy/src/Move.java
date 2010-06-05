// Move.java
// All code is work of Jennifer Freeman, 2001
import java.lang.Comparable;
public class Move implements Comparable
{   
    private String country;
    private Port portFrom;
    private Port portTo;
    public Move(String count, String from,String to)
    {
      portFrom=new Port(from);
      portTo=new Port(to);
      country=count;
    }
    public Port getPortFrom()
    {
      return portFrom;
    }
    public Port getPortTo()
    {
      return portTo;
    }
    public String getCountry()
    {
      return country;
    }
    public int compareTo(Object b)
    { 
      if (b instanceof Move)
      {
        return country.compareTo(((Move)b).getCountry());
      }
      else
      {
       return country.compareTo((String) b);
      }
    }
    public boolean equals(Object b)
    {
      return country.equals(b);
    }
    public String toString()
    {
      return Map.getCountry(country).getName()+" From:"+portFrom+" To:"+portTo;
    }
}
