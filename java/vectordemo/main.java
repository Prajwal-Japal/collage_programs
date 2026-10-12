import java.util.*;
public class VectorDemo {
    public static void main(String[] args) {
        Vector<Integer> v = new Vector<>(7,1);
        System.out.println("Initial size of vector list is " + v.size());
        System.out.println("Initial size of vector list is " + v.capacity());
        v.add(1);
        v.add(2);
        v.add(3);
        v.add(4);
        v.add(5);
        v.add(6);
        System.out.println(" size of vector list after addition is " + v.size());
        System.out.println("capapcity of vector list is " + v.capacity());
        System.out.println("index of element is : "+  v.indexOf(4));
        v.addElement(3);
        
        System.out.println("contents of vector list is " + v);

    }
}