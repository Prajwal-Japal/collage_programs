class BankAccount {

    private int accountNo;
    private String accountHolderName;
    private double accountBalance;

    // Constructor
    BankAccount(int accountNo, String accountHolderName, double accountBalance) {
        this.accountNo = accountNo;
        this.accountHolderName = accountHolderName;
        this.accountBalance = accountBalance;
    }

    // Synchronized deposit method
    public synchronized void deposit(String userName, double amount) {

        if (amount <= 0) {
            System.out.println(userName + ": Invalid deposit amount");
            return;
        }
        accountBalance += amount;
        System.out.println(userName + " deposited " + amount);
        System.out.println("Balance: " + accountBalance);
    }
    // Synchronized withdraw method
    public synchronized void withdraw(String userName, double amount) {
        if (amount <= 0) {
            System.out.println(userName + ": Invalid withdrawal amount");
            return;     
        }
        if (amount > accountBalance) {
            System.out.println(userName + ": Insufficient balance");
            return;
        }
        accountBalance -= amount;
        System.out.println(userName + " withdrew " + amount);
        System.out.println("Balance: " + accountBalance);
    }
    // Display account details
    public synchronized void displayBalance() {
        System.out.println("Account No: " + accountNo);
        System.out.println("Account Holder: " + accountHolderName);
        System.out.println("Account Balance: " + accountBalance);
    }
}


class UserTransaction extends Thread {

    private BankAccount account;
    private String userName;
    private String transactionType;
    private double amount;

    UserTransaction(BankAccount account, String userName,
                    String transactionType, double amount) {

        this.account = account;
        this.userName = userName;
        this.transactionType = transactionType;
        this.amount = amount;
    }

    public void run() {

        if (transactionType.equalsIgnoreCase("deposit")) {
            account.deposit(userName, amount);
        }
        else if (transactionType.equalsIgnoreCase("withdraw")) {
            account.withdraw(userName, amount);
        }
    }
}


public class SynchronizationPracticum {

    public static void main(String[] args) throws InterruptedException {

        BankAccount account =
                new BankAccount(101, "Pajju", 5000);

        Thread user1 =
                new UserTransaction(account, "User A", "deposit", 2000);

        Thread user2 =
                new UserTransaction(account, "User B", "withdraw", 3000);

        user1.start();
        user2.start();

        user1.join();
        user2.join();

        System.out.println("\nFinal Account Details:");
        account.displayBalance();
    }
}