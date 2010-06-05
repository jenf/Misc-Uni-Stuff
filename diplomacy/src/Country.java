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
    private TreeSet links;
    private boolean checked;
    public Country (String lon, String pa,String pb, LandType t,boolean sup)
    {
        longname=lon;
        porta=pa;
        portb=pb;
        type=t;
        supply=sup;
        links=new TreeSet();
    }
    // Consistancy Checker
    public void check()
    {
        Iterator g;
        TreeSet foo=new TreeSet();
        g=links.iterator();
        checked=true;
        while (g.hasNext())
        {
          Move a;
          Country bar;
          a=(Move) g.next();
          bar=Map.getCountry(a.getCountry());
          if (bar==null) {System.out.println("Bad Entry in "+longname+a);}
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
    public TreeSet getFleetMoves(Port g)
    {
        Iterator b;
        TreeSet myseas, a=new TreeSet();
        if (type.isLand()) return a;
        myseas=this.getSeas(g);
        b=myseas.iterator();
        while (b.hasNext())
        {
            Country s;
            TreeSet seas;
            LandType f;
            Iterator bob;
            String y;
            Move x;
            x=(Move) b.next();
            y=x.getCountry();
            s=Map.getCountry(y);
            seas=s.getSeasReverse(g);
            f=s.getType();
            if (f.isCoast())
            {
              seas.retainAll(myseas);
              if (seas.size()>0) a.add(x);
            }
            else
            {
              if (f.isSea()) a.add(x);
            }
        }
        return a;
    }
   // Get army moves.
   public TreeSet getArmyMoves()
   {
        Iterator g;
        TreeSet foo=new TreeSet();
        if (type.isSea()) return foo;
        g=links.iterator();
        while (g.hasNext())
        {
           Move x=(Move) g.next();
           String y=x.getCountry();
           LandType f;
           Country s;
           s=Map.getCountry(y);
           f=s.getType();
           if (f.isCoast() || f.isLand()) foo.add(x);
         }
         return foo;
    }
    // Get seas we can move from.
    public TreeSet getSeas(Port gool)
    {
        Iterator g;
        TreeSet foo=new TreeSet();
        g=links.iterator();
        while (g.hasNext())
        {
           Move x=(Move) g.next();
           if ((gool.equals(x.getPortFrom()))==true)
           { 
             String y=(String) x.getCountry();
             LandType f;
             Country s;
             s=Map.getCountry(y);
             f=s.getType();
             if (f.isSea() || f.isCoast()) foo.add(x);
            }
         }
         return foo;
    }
    // Get seas we can move to.
    public TreeSet getSeasReverse(Port gool)
    {
        Iterator g;
        TreeSet foo=new TreeSet();
        g=links.iterator();
        while (g.hasNext())
        {
           Move x=(Move) g.next();
           if ((gool.equals(x.getPortTo()))==true)
           { 
             String y=(String) x.getCountry();
             LandType f;
             Country s;
             s=Map.getCountry(y);
             f=s.getType();
             if (f.isSea() || f.isCoast()) foo.add(x);
            }
         }
         return foo;
    }

    // This gives a list of all the countries it links to as well.
    public String getFullDetails()
    {
        String result;
        Iterator bar;
        result=this.toString()+"\n";
        bar=links.iterator();
        while (bar.hasNext())
        {            
            Move x;
            x=(Move) bar.next();
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