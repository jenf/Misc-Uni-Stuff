// Country.java
// All code is work of Jennifer Freeman, 2001
import java.util.TreeSet;
import java.util.Iterator;
import java.util.HashSet;
public class Country
{
    private String longname;
    private String porta;
    private String portb;
    private LandType type;
    private boolean supply;
    private TreeSet<Move> links;
    private boolean checked;
    public Country (String lon, String pa,String pb, LandType t,boolean sup)
    {
        longname=lon;
        porta=pa;
        portb=pb;
        type=t;
        supply=sup;
        links=new TreeSet<Move>();
    }
    // Consistancy Checker
    public void check()
    {
      checked = true;
      for (Move a : links)
      {
        Country bar;
        bar=Map.getCountry(a.getCountry());
        if (bar==null)
        {
         System.out.println("Bad Entry in "+longname+a);
        }
        else
        {
          if (bar.check_foo()==false) bar.check();
        }
      }
    }
    public boolean check_foo()
    {               
        return checked; 
    }
    public String getName()
    { 
       return longname;
    }
    public LandType getType()
    {
       return type;
    }
    public void addNeighbour(Move neighbour)
    {
        links.add(neighbour);
    }
    public TreeSet<Move> getFleetMoves(Port g)
    {
        TreeSet<Move> myseas, a=new TreeSet<Move>();
      
        /* Unable to have a fleet when it is a landlocked square */
        if (type==LandType.Land) return a;
      
        myseas=this.getSeas(g);
        for (Move x : myseas)
        {
            Country s;
            TreeSet<Move> seas;
            LandType f;
            String y;
            y=x.getCountry();
            s=Map.getCountry(y);
            seas=s.getSeasReverse(g);
            f=s.getType();
            if (f==LandType.Coast)
            {
              seas.retainAll(myseas);
              if (seas.size()>0) a.add(x);
            }
            else
            {
              if (f==LandType.Sea) a.add(x);
            }
        }
        return a;
    }
  
   // Get army moves.
   public TreeSet<Move> getArmyMoves()
   {
        TreeSet<Move> foo=new TreeSet<Move>();
     
        /* Cannot have army moves on the sea */
        if (type==LandType.Sea) return foo;
     
        for (Move x : links)
        {
           String y=x.getCountry();
           LandType f;
           Country s;
           s=Map.getCountry(y);
           f=s.getType();
           if (f==LandType.Coast || f==LandType.Land)
           {
        	   foo.add(x);
           }
         }
     
         return foo;
    }
    // Get seas we can move from.
    public TreeSet<Move> getSeas(Port gool)
    {
        TreeSet<Move> foo=new TreeSet<Move>();
        for (Move x : links)
        {
           if ((gool.equals(x.getPortFrom()))==true)
           { 
             String y=x.getCountry();
             LandType f;
             Country s;
             s=Map.getCountry(y);
             f=s.getType();
             if (f==LandType.Sea || f==LandType.Coast)
             {
            	 foo.add(x);
             }
            }
         }
         return foo;
    }
    // Get seas we can move to.
    public TreeSet<Move> getSeasReverse(Port gool)
    {
        TreeSet<Move> foo=new TreeSet<Move>();
        for (Move x : links)
        {
           if ((gool.equals(x.getPortTo()))==true)
           { 
             String y=x.getCountry();
             LandType f;
             Country s;
             s=Map.getCountry(y);
             f=s.getType();
             if (f==LandType.Sea || f==LandType.Coast)
             {
            	 foo.add(x);
             }
            }
         }
         return foo;
    }

    // This gives a list of all the countries it links to as well.
    public String getFullDetails()
    {
        String result;
        result=this.toString()+"\n";
        for (Move x : links)
        { 
            result+="   "+(Map.getCountry(x.getCountry()))+"\n";
        }
        return result;
    }
    // This just gives the country and it's details
    // (but not links, to stop recursive infinite loops)
    public String toString()
    {     
        String bar="",ports="";
        if (supply) bar="supply"; else bar="no supply";
        if (porta!=null) ports=porta;
        if (porta!=null && portb!=null) ports+=",";
        if (portb!=null) ports+=portb;
        return longname+" ("+type+","+bar+","+ports+")";
    }
}
