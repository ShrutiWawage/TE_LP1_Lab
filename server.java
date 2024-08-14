import java.io.*;
import java.net.*;
import java.util.concurrent.*;

public class Server {
    private static final int PORT = 12345;
    private static final ExecutorService threadPool = Executors.newCachedThreadPool();

    public static void main(String[] args) {
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            System.out.println("Server listening on port " + PORT);

            while (true) {
                Socket clientSocket = serverSocket.accept();
                threadPool.execute(new ClientHandler(clientSocket));
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

class ClientHandler implements Runnable {
    private Socket clientSocket;

    public ClientHandler(Socket socket) {
        this.clientSocket = socket;
    }

    @Override
    public void run() {
        try (BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
             PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true)) {

            String request;
            while ((request = in.readLine()) != null) {
                String response = processRequest(request);
                out.println(response);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private String processRequest(String request) {
        String[] parts = request.split(" ");
        if (parts.length < 2) {
            return "Error: Invalid request";
        }

        String operation = parts[0];
        try {
            double result = 0;
            double operand1, operand2;

            switch (operation) {
                case "1": // ADD
                    operand1 = Double.parseDouble(parts[1]);
                    operand2 = Double.parseDouble(parts[2]);
                    result = operand1 + operand2;
                    break;
                case "2": // SUB
                    operand1 = Double.parseDouble(parts[1]);
                    operand2 = Double.parseDouble(parts[2]);
                    result = operand1 - operand2;
                    break;
                case "3": // MUL
                    operand1 = Double.parseDouble(parts[1]);
                    operand2 = Double.parseDouble(parts[2]);
                    result = operand1 * operand2;
                    break;
                case "4": // DIV
                    operand1 = Double.parseDouble(parts[1]);
                    operand2 = Double.parseDouble(parts[2]);
                    if (operand2 != 0) {
                        result = operand1 / operand2;
                    } else {
                        return "Error: Division by zero";
                    }
                    break;
                case "5": // SQRT
                    operand1 = Double.parseDouble(parts[1]);
                    result = Math.sqrt(operand1);
                    break;
                case "6": // POW
                    operand1 = Double.parseDouble(parts[1]);
                    operand2 = Double.parseDouble(parts[2]);
                    result = Math.pow(operand1, operand2);
                    break;
                default:
                    return "Error: Unknown operation";
            }
            return "Result: " + result;
        } catch (NumberFormatException e) {
            return "Error: Invalid number format";
        } catch (ArrayIndexOutOfBoundsException e) {
            return "Error: Missing operands";
        }
    }
}