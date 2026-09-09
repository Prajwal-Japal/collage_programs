class MCatch{
    public static void main (String[] args){
        try{
            int a = args.length;
            int b = 42/a;
            int c[] = {1};
            c[42] = 99;
        }catch(ArithmeticException e){
            System.out.println("Divide by 0: " + e);
        }catch(ArrayIndexOutOfBoundsException e){
            System.out.println("Array index out of bounds: " + e);
        }
        System .out.println("After try/catch blocks.");
    }
}