class declare{
    int a,b,c,x;
    declare(int a,int b,int c, int x){
        this.a=a;
        this.b=b;
        this.c=c;
        this.x=x;
    }

    void calculate() {
        double d=Math.sqrt(b*b-4*a*c);
        if (d==0){
            System.out.println("Roots are real and the same.");
            double root=-b/(2*a);
            System.out.println("the root is"+root);
        }else if(d<0){
            System.out.println("Roots are complex and different.");
        }else if(d>0){
            System.out.println("Roots are real and different.");
        }
    }

    void roots(){
        double root1=(-b+Math.sqrt(b*b-4*a*c))/(2*a);
        double root2=(-b-Math.sqrt(b*b-4*a*c))/(2*a);
        System.out.println("Root 1: " + root1);
        System.out.println("Root 2: " + root2);
    }
}

class Main{
    public static void main(String[] args) {
        declare obj = new declare(1, -3, 2, 0);   
        obj.calculate();
        obj.roots();
    }
}