import Partial;
import java.util.Collection;
import java.util.Iterator;
public abstract class List
{                                               
   static public int multiply(int a,int b)
   {
       return (a*b);
   }
   public static Object product(Collection in) throws java.lang.ClassNotFoundException,java.lang.NoSuchMethodException,IllegalAccessException, java.lang.reflect.InvocationTargetException
   {
     Partial current;
     return List.foldl(in,new Integer(1),new Partial("multiply","List"));
   }
   static public int add(int a,int b)
   {
       return (a+b);
   }
   public static Integer sum(Collection in) throws java.lang.ClassNotFoundException,java.lang.NoSuchMethodException,IllegalAccessException, java.lang.reflect.InvocationTargetException
   {
     Partial current;
     return (Integer) (List.foldl(in,new Integer(0),new Partial("add","List")));
   }
   public static Collection filter(Collection in, Collection out,Partial f) throws java.lang.ClassNotFoundException,java.lang.NoSuchMethodException,IllegalAccessException, java.lang.reflect.InvocationTargetException
   {
     Partial current;
     current=new Partial("filter_single","List");
     current=current.addArg(f);
     return (Collection) foldl(in,out,current);
   }
   public static Collection filter_single (Partial f,Object thisone,Object out) throws java.lang.ClassNotFoundException,java.lang.NoSuchMethodException,IllegalAccessException, java.lang.reflect.InvocationTargetException
   {
     Partial current;
     Object k;
     Collection bar=(Collection) out;
     k=f.addArgAndRun(thisone);
     if (((Boolean) k).booleanValue()) {bar.add(thisone);}
     return bar;
   }
   public static Collection map(Collection in, Collection out,Partial f) throws java.lang.ClassNotFoundException,java.lang.NoSuchMethodException,IllegalAccessException, java.lang.reflect.InvocationTargetException
   {
     Partial current;
     current=new Partial("map_single","List");
     current=current.addArg(f);
     return (Collection) foldl(in,out,current);
   }
   public static Collection map_single (Partial f,Object thisone,Object out) throws java.lang.ClassNotFoundException,java.lang.NoSuchMethodException,IllegalAccessException, java.lang.reflect.InvocationTargetException
   {
     Partial current;
     Object k;
     Collection bar=(Collection) out;
     bar.add(f.addArgAndRun(thisone));
     return bar;
   }
   public static Object foldl (Collection in, Object out, Partial f) throws java.lang.ClassNotFoundException,java.lang.NoSuchMethodException,IllegalAccessException, java.lang.reflect.InvocationTargetException
   {
     Iterator g;
     g=in.iterator();
     while (g.hasNext())
     {
        Object i;
        i=g.next();
        out=(f.addArg(i)).addArgAndRun(out);
     }
     return out;
   }
}
