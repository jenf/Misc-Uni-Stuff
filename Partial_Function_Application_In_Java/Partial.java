import java.lang.Class;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.IllegalAccessException; 
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Field;
public class Partial
{       
       String method;
       Class class_to_run;
       Object lastarg;
       Object[] args;
       public Partial(String meth)
       { 
         method=meth;
         lastarg=null;
         class_to_run=null;
         args=new Object[0];
       }
       public Partial(String meth,String class_to) throws ClassNotFoundException
       { 
         method=meth;
         lastarg=null;
         class_to_run=Class.forName(class_to);
         args=new Object[0];
       }
       private Partial(Object foo, String meth,Object[] arg,Class class_to)
       {
         method=meth;
         args=arg;
         lastarg=foo;
         class_to_run=class_to;
       }
       public Partial addArg(int arg)
       { return addArg(new Integer(arg));}
       public Partial addArg(float arg)
       { return addArg(new Float(arg));}
       public Partial addArg(double arg)
       { return addArg(new Double(arg));}
       public Partial addArg(boolean arg)
       { return addArg(new Boolean(arg));}
       public Partial addArg(Object arg)
       {  
          Object[] foo=args;
          if (lastarg!=null)
          {
            foo=new Object[args.length+1];
            for (int i=0;i<args.length;i++)
            {
              foo[i]=args[i];
            }
            foo[args.length]=lastarg;
          }
          return new Partial(arg,method,foo,class_to_run);
       }
       public Object addArgAndRun(Object arg) throws ClassNotFoundException, NoSuchMethodException, IllegalAccessException, InvocationTargetException
       {  
         return this.addArg(arg).Run();
       }
       public Object[] addLast()
       {  
          Object[] foo=args;
          if (lastarg!=null)
          {
            foo=new Object[args.length+1];
            for (int i=0;i<args.length;i++)
            {
              foo[i]=args[i];
            }
            foo[args.length]=lastarg;
          }
          return foo;
       }
       public Object Run() throws ClassNotFoundException, NoSuchMethodException, IllegalAccessException, InvocationTargetException
       {
            if (class_to_run==null) return this.RunClass();
            return this.RunAsStatic();
       }
       public Object RunClass() throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
       {     
            Class foo;
            Method bar;
            foo=lastarg.getClass();
            bar=this.find_Method(foo);
            return bar.invoke(lastarg,args);
       }
       public Object RunAsStatic() throws NoSuchMethodException, IllegalAccessException, InvocationTargetException
       {     
            Method bar;
            Object[] args_new=addLast();
            bar=this.find_Method_Static(class_to_run,args_new);
            if (bar==null) throw new NoSuchMethodException();
            return bar.invoke(null,args_new);
       }
       public Method find_Method(Class foo)
       {
         Method[] bar;
         bar=foo.getMethods();
         for (int i=0;i<bar.length;i++)
         {   
            if (method.equals(bar[i].getName()))
            {   
                if (check_Method(bar[i].getParameterTypes(),args)) return bar[i];
            }
         }
         System.out.println("Cannot find method");
         return null;
       }
       public Method find_Method_Static(Class foo,Object[] args_)
       {
         Method[] bar;
         bar=foo.getMethods();
         for (int i=0;i<bar.length;i++)
         {   
            if (method.equals(bar[i].getName()))
            {   
                if (Modifier.isStatic(bar[i].getModifiers()))
                {if (check_Method(bar[i].getParameterTypes(),args_)) return bar[i];}
            }
         }
         System.out.println("Cannot find method "+method +" in " + foo);
         return null;
       }
       public boolean check_Method(Class[] todo,Object[] args_)
       {
          if (todo.length!=args_.length) return false;
          for (int i=0;i<todo.length;i++)
          {
               check_type(todo[i],args_[i]);
          }
          return true;
       }
       public static boolean check_type(Class todo,Object args)
       {  
        return (todo.isInstance(args));
       }
}

