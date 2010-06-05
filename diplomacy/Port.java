// Port.java
// All code is work of Jennifer Freeman, 2001
public class Port 
{
    private int port;
    public static final Port PortA=new Port ("a");
    public static final Port PortB=new Port ("b");
    public static final Port NoPort=new Port (null);

    public Port (String b)
    {   
        if (b==null) {port=3;return;}
        String t=b.toLowerCase();
        if (t.equals("a"))
        { port=1; } else
        if (t.equals("b"))
        { port=2; } else
        { port=3; }
    }
    public boolean isPortA()
    {
        return port==1;
    }
    public boolean isPortB()
    { 	
        return port==2;
    }
    public boolean isNoPort()
    {
        return port==3;
    }
    public boolean equals(Object bob)
    {   
        Port gid;
        gid=(Port) bob;
        if (port==3 || gid.port==3) return true;
        return (port==(gid.port));
    }
    public String toString()
    {
        if (isPortA()) return "port A";
        if (isPortB()) return "port B";
        return "No Port";
    }
}
