import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.awt.*;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.List;

public class TlipGui extends JFrame {

    private JRadioButton rbSingle, rbBatch;
    private JPanel centerPanel;
    private CardLayout centerCardLayout;

    // Single File Components
    private JTextField tfSingleInput;
    private JTextField tfSingleWidth, tfSingleHeight, tfSingleSize;
    private JTextField tfSingleOutput;

    // Batch Components
    private JTextField tfBatchInputDir;
    private JRadioButton rbBatchModeBatch, rbBatchModeIndividual;
    private JPanel batchSettingsPanel; // Holds W/H/Size for batch mode
    private JTextField tfBatchWidth, tfBatchHeight, tfBatchSize;
    private JTextField tfBatchOutputDir;

    // Common
    private JTextArea logArea;
    private JButton btnProcess;

    private static final String TLIP_BINARY = "../tlip";

    public TlipGui() {
        setTitle("Tlip GUI Wrapper");
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setSize(600, 700);
        setLocationRelativeTo(null);

        initComponents();
    }

    private void initComponents() {
        Container contentPane = getContentPane();
        contentPane.setLayout(new BorderLayout(10, 10));

        // --- Top Panel: Mode Selection ---
        JPanel topPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        topPanel.setBorder(new EmptyBorder(10, 10, 0, 10));
        topPanel.add(new JLabel("Processing mode: "));
        rbSingle = new JRadioButton("Single file", true);
        rbBatch = new JRadioButton("Batch");
        ButtonGroup modeGroup = new ButtonGroup();
        modeGroup.add(rbSingle);
        modeGroup.add(rbBatch);
        topPanel.add(rbSingle);
        topPanel.add(rbBatch);
        contentPane.add(topPanel, BorderLayout.NORTH);

        // --- Center Panel: Dynamic Content ---
        centerCardLayout = new CardLayout();
        centerPanel = new JPanel(centerCardLayout);
        centerPanel.setBorder(new EmptyBorder(10, 20, 10, 20));

        // 1. Single File Panel
        JPanel singlePanel = createSingleFilePanel();
        centerPanel.add(singlePanel, "SINGLE");

        // 2. Batch Panel
        JPanel batchPanel = createBatchPanel();
        centerPanel.add(batchPanel, "BATCH");

        contentPane.add(centerPanel, BorderLayout.CENTER);

        // --- Bottom Panel: Process & Log ---
        JPanel bottomPanel = new JPanel(new BorderLayout(5, 5));
        bottomPanel.setBorder(new EmptyBorder(0, 10, 10, 10));

        btnProcess = new JButton("Process");
        btnProcess.setFont(new Font("Dialog", Font.BOLD, 14));
        bottomPanel.add(btnProcess, BorderLayout.NORTH);

        logArea = new JTextArea(10, 50);
        logArea.setEditable(false);
        logArea.setFont(new Font("Monospaced", Font.PLAIN, 12));
        JScrollPane scrollPane = new JScrollPane(logArea);
        bottomPanel.add(scrollPane, BorderLayout.CENTER);

        contentPane.add(bottomPanel, BorderLayout.SOUTH);

        // --- Listeners ---
        rbSingle.addActionListener(e -> centerCardLayout.show(centerPanel, "SINGLE"));
        rbBatch.addActionListener(e -> centerCardLayout.show(centerPanel, "BATCH"));

        btnProcess.addActionListener(e -> startProcessing());
    }

    private JPanel createSingleFilePanel() {
        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

        // Input File
        panel.add(createLabel("Input File:"));
        JPanel inputPanel = new JPanel(new BorderLayout(5, 0));
        tfSingleInput = new JTextField();
        JButton btnBrowseInput = new JButton("Browse");
        btnBrowseInput.addActionListener(e -> chooseFile(tfSingleInput, false));
        inputPanel.add(tfSingleInput, BorderLayout.CENTER);
        inputPanel.add(btnBrowseInput, BorderLayout.EAST);
        inputPanel.setMaximumSize(new Dimension(Integer.MAX_VALUE, 30));
        panel.add(inputPanel);
        panel.add(Box.createVerticalStrut(10));

        // Settings
        tfSingleWidth = new JTextField();
        tfSingleHeight = new JTextField();
        tfSingleSize = new JTextField();
        panel.add(createSettingsPanel(tfSingleWidth, tfSingleHeight, tfSingleSize));
        panel.add(Box.createVerticalStrut(10));

        // Output File
        panel.add(createLabel("Output File:"));
        JPanel outputPanel = new JPanel(new BorderLayout(5, 0));
        tfSingleOutput = new JTextField();
        JButton btnBrowseOutput = new JButton("Browse");
        btnBrowseOutput.addActionListener(e -> chooseFile(tfSingleOutput, true)); // Save dialog? Or just file chooser
        outputPanel.add(tfSingleOutput, BorderLayout.CENTER);
        outputPanel.add(btnBrowseOutput, BorderLayout.EAST);
        outputPanel.setMaximumSize(new Dimension(Integer.MAX_VALUE, 30));
        panel.add(outputPanel);

        panel.add(Box.createVerticalGlue());
        return panel;
    }

    private JPanel createBatchPanel() {
        JPanel panel = new JPanel();
        panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

        // Input Directory
        panel.add(createLabel("Input Directory:"));
        JPanel inputPanel = new JPanel(new BorderLayout(5, 0));
        tfBatchInputDir = new JTextField();
        JButton btnBrowseInput = new JButton("Browse");
        btnBrowseInput.addActionListener(e -> chooseDirectory(tfBatchInputDir));
        inputPanel.add(tfBatchInputDir, BorderLayout.CENTER);
        inputPanel.add(btnBrowseInput, BorderLayout.EAST);
        inputPanel.setMaximumSize(new Dimension(Integer.MAX_VALUE, 30));
        panel.add(inputPanel);
        panel.add(Box.createVerticalStrut(10));

        // Batch Mode Sub-selection
        JPanel radioPanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        rbBatchModeBatch = new JRadioButton("Batch input", true);
        rbBatchModeIndividual = new JRadioButton("Individual input");
        ButtonGroup group = new ButtonGroup();
        group.add(rbBatchModeBatch);
        group.add(rbBatchModeIndividual);
        radioPanel.add(rbBatchModeBatch);
        radioPanel.add(rbBatchModeIndividual);
        radioPanel.setAlignmentX(Component.LEFT_ALIGNMENT);
        panel.add(radioPanel);

        // Settings (Conditional)
        tfBatchWidth = new JTextField();
        tfBatchHeight = new JTextField();
        tfBatchSize = new JTextField();
        batchSettingsPanel = createSettingsPanel(tfBatchWidth, tfBatchHeight, tfBatchSize);
        panel.add(batchSettingsPanel);
        
        // Toggle settings visibility
        ActionListener toggleListener = e -> batchSettingsPanel.setVisible(rbBatchModeBatch.isSelected());
        rbBatchModeBatch.addActionListener(toggleListener);
        rbBatchModeIndividual.addActionListener(toggleListener);

        panel.add(Box.createVerticalStrut(10));

        // Output Directory
        panel.add(createLabel("Output Directory:"));
        JPanel outputPanel = new JPanel(new BorderLayout(5, 0));
        tfBatchOutputDir = new JTextField();
        JButton btnBrowseOutput = new JButton("Browse");
        btnBrowseOutput.addActionListener(e -> chooseDirectory(tfBatchOutputDir));
        outputPanel.add(tfBatchOutputDir, BorderLayout.CENTER);
        outputPanel.add(btnBrowseOutput, BorderLayout.EAST);
        outputPanel.setMaximumSize(new Dimension(Integer.MAX_VALUE, 30));
        panel.add(outputPanel);

        panel.add(Box.createVerticalGlue());
        return panel;
    }

    private JPanel createSettingsPanel(JTextField w, JTextField h, JTextField s) {
        JPanel panel = new JPanel(new GridLayout(3, 2, 5, 5));
        panel.setBorder(BorderFactory.createTitledBorder("Settings"));
        panel.setMaximumSize(new Dimension(Integer.MAX_VALUE, 120));
        
        panel.add(new JLabel("Width (px):"));
        panel.add(w);
        panel.add(new JLabel("Height (px):"));
        panel.add(h);
        panel.add(new JLabel("Max Size (KB):"));
        panel.add(s);
        
        return panel;
    }

    private JLabel createLabel(String text) {
        JLabel label = new JLabel(text);
        label.setAlignmentX(Component.LEFT_ALIGNMENT);
        return label;
    }

    private void chooseFile(JTextField target, boolean save) {
        JFileChooser chooser = new JFileChooser();
        if (save) chooser.setDialogType(JFileChooser.SAVE_DIALOG);
        if (chooser.showDialog(this, "Select") == JFileChooser.APPROVE_OPTION) {
            target.setText(chooser.getSelectedFile().getAbsolutePath());
        }
    }

    private void chooseDirectory(JTextField target) {
        JFileChooser chooser = new JFileChooser();
        chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        if (chooser.showDialog(this, "Select Directory") == JFileChooser.APPROVE_OPTION) {
            target.setText(chooser.getSelectedFile().getAbsolutePath());
        }
    }

    private void startProcessing() {
        logArea.setText("");
        if (rbSingle.isSelected()) {
            processSingle();
        } else {
            if (rbBatchModeBatch.isSelected()) {
                processBatchBatch();
            } else {
                processBatchIndividual();
            }
        }
    }

    private void processSingle() {
        String input = tfSingleInput.getText();
        String width = tfSingleWidth.getText();
        String height = tfSingleHeight.getText();
        String size = tfSingleSize.getText();
        String output = tfSingleOutput.getText();

        if (input.isEmpty() || output.isEmpty()) {
            log("Error: Input and Output paths are required.");
            return;
        }

        new SwingWorker<Void, String>() {
            @Override
            protected Void doInBackground() throws Exception {
                runTlipCommand(input, width, height, size, output, true);
                return null;
            }
        }.execute();
    }

    private void processBatchBatch() {
        String inputDir = tfBatchInputDir.getText();
        String width = tfBatchWidth.getText();
        String height = tfBatchHeight.getText();
        String size = tfBatchSize.getText();
        String outputDir = tfBatchOutputDir.getText();

        if (inputDir.isEmpty() || outputDir.isEmpty()) {
            log("Error: Input and Output directories are required.");
            return;
        }

        new SwingWorker<Void, String>() {
            @Override
            protected Void doInBackground() throws Exception {
                // Use tlip's batch mode
                // ./tlip --cli <src> <width> <height> <size> <dest> not-individual
                runTlipCommand(inputDir, width, height, size, outputDir, false);
                return null;
            }
        }.execute();
    }

    private void processBatchIndividual() {
        String inputDirStr = tfBatchInputDir.getText();
        String outputDirStr = tfBatchOutputDir.getText();

        if (inputDirStr.isEmpty() || outputDirStr.isEmpty()) {
            log("Error: Input and Output directories are required.");
            return;
        }

        File inputDir = new File(inputDirStr);
        File[] files = inputDir.listFiles((dir, name) -> {
            String lower = name.toLowerCase();
            return lower.endsWith(".jpg") || lower.endsWith(".jpeg");
        });

        if (files == null || files.length == 0) {
            log("No JPEG files found in input directory.");
            return;
        }

        new SwingWorker<Void, String>() {
            @Override
            protected Void doInBackground() throws Exception {
                int total = files.length;
                for (int i = 0; i < total; i++) {
                    File file = files[i];
                    final int index = i + 1;
                    
                    // Show Dialog on EDT and wait for result
                    final String[] params = new String[3]; // w, h, size
                    final boolean[] proceed = {false};

                    SwingUtilities.invokeAndWait(() -> {
                        IndividualProcessDialog dialog = new IndividualProcessDialog(TlipGui.this, file.getName(), index, total);
                        dialog.setVisible(true);
                        if (dialog.isConfirmed()) {
                            params[0] = dialog.getWidthVal();
                            params[1] = dialog.getHeightVal();
                            params[2] = dialog.getSizeVal();
                            proceed[0] = true;
                        }
                    });

                    if (!proceed[0]) {
                        publish("Processing cancelled by user at " + file.getName());
                        break;
                    }

                    String output = new File(outputDirStr, file.getName()).getAbsolutePath();
                    runTlipCommand(file.getAbsolutePath(), params[0], params[1], params[2], output, true);
                }
                return null;
            }

            @Override
            protected void process(List<String> chunks) {
                for (String s : chunks) log(s);
            }
        }.execute();
    }

    private void runTlipCommand(String input, String w, String h, String s, String output, boolean individual) throws IOException, InterruptedException {
        List<String> command = new ArrayList<>();
        command.add(TLIP_BINARY);
        command.add("--cli");
        command.add(input);
        command.add(w.isEmpty() ? "" : w);
        command.add(h.isEmpty() ? "" : h);
        command.add(s.isEmpty() ? "" : s);
        command.add(output);
        command.add(individual ? "individual" : "not-individual");
        command.add("no-open");

        log("Running: " + String.join(" ", command));

        ProcessBuilder pb = new ProcessBuilder(command);
        pb.redirectErrorStream(true);
        Process process = pb.start();

        // Handle potential prompts (like directory exists or overwrite) by writing "y"
        try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(process.getOutputStream()))) {
             // If tlip prompts, we blindly say yes. 
             writer.write("y\n");
             writer.flush();
        } catch (IOException ignored) {}

        try (BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()))) {
            String line;
            while ((line = reader.readLine()) != null) {
                log(line);
            }
        }

        int exitCode = process.waitFor();
        log("Process exited with code " + exitCode);
        log("--------------------------------------------------");
    }

    private void log(String message) {
        SwingUtilities.invokeLater(() -> {
            logArea.append(message + "\n");
            logArea.setCaretPosition(logArea.getDocument().getLength());
        });
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> new TlipGui().setVisible(true));
    }

    // Inner class for the popup dialog
    class IndividualProcessDialog extends JDialog {
        private JTextField tfW, tfH, tfS;
        private boolean confirmed = false;

        public IndividualProcessDialog(Frame owner, String fileName, int current, int total) {
            super(owner, "Process Image " + current + "/" + total, true);
            setLayout(new BorderLayout(10, 10));
            setSize(300, 250);
            setLocationRelativeTo(owner);

            JPanel top = new JPanel();
            top.add(new JLabel("Processing: " + fileName));
            top.add(new JLabel("(" + current + "/" + total + ")"));
            add(top, BorderLayout.NORTH);

            tfW = new JTextField();
            tfH = new JTextField();
            tfS = new JTextField();
            add(createSettingsPanel(tfW, tfH, tfS), BorderLayout.CENTER);

            JButton btnOk = new JButton("OK");
            btnOk.addActionListener(e -> {
                confirmed = true;
                dispose();
            });
            JPanel bottom = new JPanel();
            bottom.add(btnOk);
            add(bottom, BorderLayout.SOUTH);
        }

        public boolean isConfirmed() { return confirmed; }
        public String getWidthVal() { return tfW.getText(); }
        public String getHeightVal() { return tfH.getText(); }
        public String getSizeVal() { return tfS.getText(); }
    }
}
