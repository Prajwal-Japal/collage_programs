import java.util.Scanner;

class Payment {
    void makePayment(double amount) {
        System.out.println("Base Payment of :" + amount + " made");
    }
    void makePayment(double amount, String transactionId) {
        System.out.println("Base Payment of :" + amount + " made using " + transactionId);
    }
}

class CreditCardPayment extends Payment {
    @Override
    void makePayment(double amount) {
        System.out.println("Credit Card Payment of :" + amount + " made");
    }
}

class UPIPayment extends Payment {
    @Override
    void makePayment(double amount) {
        System.out.println("UPI Payment of :" + amount + " made");
    }
}

class NetBankingPayment extends Payment {
    @Override
    void makePayment(double amount) {
        System.out.println("Net Banking Payment of :" + amount + " made");
    }
}

public class pract4 {
    public static void main(String[] args) {
        Scanner sc = new Scanner(System.in);
        System.out.println("Enter the amount for Credit Card Payment:");
        double creditCA = sc.nextDouble();
        System.out.println("Enter the amount for UPI Payment:");
        double Upi = sc.nextDouble();
        System.out.println("Enter the amount for Net Banking Payment:");
        double NetBank = sc.nextDouble();
        System.out.println("Enter the Transaction ID:");
        String ID = sc.next();
        System.out.println("Enter the amount for Base Payment:");
        double a=sc.nextDouble();

        if (a < 0) {
            System.out.println("Invalid input");
            return;
        } else

        if (creditCA < 0 || Upi < 0 || NetBank < 0) {
            System.out.println("Invalid input");
            return;
        }else if (creditCA == 0 && Upi == 0 && NetBank == 0) {
            System.out.println("No payment made");
            return;
        } else if (ID == null || ID.trim().isEmpty()) {
            System.out.println("Transaction ID cannot be empty");
            return;
        } else {
            Payment payment= new Payment();
            payment.makePayment(a);
            payment.makePayment(a, ID);

            Payment card = new CreditCardPayment();
            Payment upi = new UPIPayment();
            Payment netBank = new NetBankingPayment();

            card.makePayment(creditCA);
            upi.makePayment(Upi);
            netBank.makePayment(NetBank);  
        }
        sc.close();      
    }
}

