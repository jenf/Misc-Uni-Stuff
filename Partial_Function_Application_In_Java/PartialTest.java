import Partial;
import java.util.ArrayList;
import java.util.TreeSet;
public class PartialTest  
{
    static public void main(String[] args) throws java.lang.ClassNotFoundException, java.lang.NoSuchMethodException, IllegalAccessException, java.lang.reflect.InvocationTargetException
    {
        ArrayList bob=new ArrayList();
        bob.add(new Integer(2));
        bob.add(new Integer(3));
        Partial foo=new Partial("add");
        foo=foo.addArg(-4);
        foo=foo.addArg(bob);
        foo.Run();
        for (int i=0;i<400;i++) foo.Run();
        System.out.println(bob);
        Partial okam=new Partial("even");
        System.out.println(List.map(bob,new ArrayList(),new Partial("square","PartialTest")));
        System.out.println(List.filter(bob,new ArrayList(),new Partial("even","PartialTest")));
        System.out.println(List.sum(bob));
        System.out.println(List.product(bob));
    }
    static public int square (int a)
    {
        return (a*a);
    }
    static public void moo(int a)
    {
        System.out.println(a);
    }
    static public boolean even(int a)
    {
        return (a%2==0); 
    }
}
