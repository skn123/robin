package wizard;
import org.eclipse.jface.preference.DirectoryFieldEditor;
import org.eclipse.jface.preference.FileFieldEditor;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.List;
/*
 * Created on 12/10/2005 by Misha.
 */

public class FilesPage extends WizardPage {
    public static final String DESCRIPTION_STR = "Choose the files to wrap and the output file";
    
    public FilesPage() {
        super("Files");
        this.setTitle("Files stage");
        this.setDescription(DESCRIPTION_STR);
    }
    
    public void createControl(Composite parent) {
        // Create the main composite.
        Composite composite = new Composite(parent, SWT.NONE);
        composite.setLayout(new GridLayout(1, false));
        composite.setLayoutData(new GridData(GridData.FILL_BOTH));

        // put everything into the composite.
        this.inputFiles(composite);
        new Label(composite, SWT.HORIZONTAL | SWT.SEPARATOR).setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        this.outputFile(composite);

        // set the composite to be in the page.
        this.setControl(composite);
    }
    
    /**
     * Check if all needed is filled, and set pagecomplete status to true if it does.
     */
    private void letContinue() {
        // Check if everything is loaded yet (you don't want to validate
        // continuation if things are not loaded.
        if (this.getControl() != null) {
            this.setPageComplete(this.validateInputFiles() && this.validateOutputFiles());
        }
    }
    
    /**
     * Check if the input files boxes are filled.
     * 
     * @return true if everything's ok.
     */
    private boolean validateInputFiles() {
        boolean ok = false;
        if (this.isDoxygen()) {
            ok = !this.getDoxygenXmlDir().equals("");        
        } else {
            ok = this.getSourceList().length != 0;
        }
        if (!ok) {
            this.setMessage("Input files are not set", ERROR);
            return false;
        }
        this.setMessage(DESCRIPTION_STR);
        return true;
    }
    
    /**
     * Check if the output files boxes are filled.
     * 
     * @return true if everything's ok.
     */
    private boolean validateOutputFiles() {
        if (this.getOutputFile().equals("")) {
            this.setMessage("Output file is not set", ERROR);
            return false;
        }
        this.setMessage(DESCRIPTION_STR);
        return true;
    }
    
    /**
     * Create all the fields in the input section.
     * 
     * @param parent The composite to put everything into.
     */
    private void inputFiles(Composite parent) {
        // Create the main composite for everything in this section.
        Composite composite = new Composite(parent, SWT.NONE);
        composite.setLayout(new GridLayout(1, false));
        composite.setLayoutData(new GridData(GridData.FILL_BOTH));
        
        // A description of this section.
        Label description = new Label(composite, SWT.WRAP);
        description.setText("Please choose Griffin's input:");
        description.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        
        // The radio button for Doxygen's XML output dir.
        Button xmlDirButton = new Button(composite, SWT.RADIO);
        xmlDirButton.setText("The XML output of Doxygen:");
        xmlDirButton.addSelectionListener(new SelectionListener() {
            public void widgetSelected(SelectionEvent e) {
                selectedXML(true);
            }
            // Never called.
            public void widgetDefaultSelected(SelectionEvent e) {}
        });
        // Create the selection of the XML dir.
        this.xmlOutputDir(composite);     
        // The radio button for the source file selection.
        Button sourceButton = new Button(composite, SWT.RADIO);
        sourceButton.setText("The source files:");

        sourceButton.addSelectionListener(new SelectionListener() {
            public void widgetSelected(SelectionEvent e) {
                selectedXML(false);
            }
            // Never called.
            public void widgetDefaultSelected(SelectionEvent e) {}
        });
        // Create the selection of the source files.
        this.sourceFiles(composite);
        
        // Select the XML radio button.
        xmlDirButton.setSelection(true);
        sourceButton.setSelection(false);
        this.selectedXML(true);
    }
    
    /**
     * What to do when a selection of the radio buttons changed.
     * 
     * @param b is the XML option selected?
     */
    private void selectedXML(boolean b) {
        this.doxygen = b;
        this.doxygenXmlDirField.setEnabled(b, this.doxygenXmlDirComposite);
        this.sourceList.setEnabled(!b);
        if (b) {
            this.sourceList.deselectAll();
        }
        Control[] buttons = this.sourceButtonsComposite.getChildren();
        for (int i = 0; i < buttons.length; ++i) {
            buttons[i].setEnabled(!b);
        }
        letContinue();
    }
    
    /**
     * This function creates the input field for Doxygen's XML output dir.
     * 
     * @param parent Where to put the composite.
     */
    private void xmlOutputDir(Composite parent) {
        // Create the composite for the input widget.
        this.doxygenXmlDirComposite = new Composite(parent, SWT.NONE);
        this.doxygenXmlDirComposite.setLayout(new GridLayout(1, false));
        GridData data = new GridData(GridData.FILL_HORIZONTAL);
        data.horizontalIndent = 20;
        this.doxygenXmlDirComposite.setLayoutData(data);
        
        // Create the widget itself.
        this.doxygenXmlDirField = new DirectoryFieldEditor("xmlInput", "Doxygen's XML dir:", this.doxygenXmlDirComposite);
        this.doxygenXmlDirField.setStringValue("xml\\");
        this.doxygenXmlDirField.setEmptyStringAllowed(false);
        this.doxygenXmlDirField.getTextControl(this.doxygenXmlDirComposite).addModifyListener(new ModifyListener() {
            public void modifyText(ModifyEvent e) {
                letContinue();
            }           
        });
    }
    
    private void sourceFiles(Composite parent) {
        Composite composite = new Composite(parent, SWT.NONE);
        GridLayout compositeLayout = new GridLayout(2, false);
        compositeLayout.marginHeight = 0;
        compositeLayout.marginWidth  = 0;
        composite.setLayout(compositeLayout);
        GridData data = new GridData(GridData.FILL_BOTH);
        data.horizontalIndent = 20;
        composite.setLayoutData(data);
        
        this.sourceList = new List(composite, SWT.MULTI | SWT.BORDER | SWT.H_SCROLL | SWT.V_SCROLL);
        this.sourceList.setLayoutData(new GridData(GridData.FILL_BOTH));
        
        this.sourceButtonsComposite = new Composite(composite, SWT.NONE);

        GridLayout buttonsLayout = new GridLayout(1, false);
        buttonsLayout.marginHeight = 0;
        buttonsLayout.marginWidth  = 0;
        this.sourceButtonsComposite.setLayout(buttonsLayout);
        this.sourceButtonsComposite.setLayoutData(new GridData(GridData.VERTICAL_ALIGN_BEGINNING));
        
        Button addButton = new Button(this.sourceButtonsComposite, SWT.NONE);
        addButton.setText("Add files...");
        addButton.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        addButton.addSelectionListener(new SelectionListener() {
            public void widgetSelected(SelectionEvent e) {
                String[] extensions = {"*.cc;*.cpp;*.h;*.hh;*.hpp",
                                       "*.h;*.hh;*.hpp",
                                       "*.cc;*.cpp",
                                       "*.*"};
                String[] extNames   = {"All C++ files (*.cc;*.cpp;*.h;*.hh;*.hpp)",
                                       "C++ header files (*.h;*.hh;*.hpp)",
                                       "C++ source files (*.cc;*.cpp)",
                                       "All files (*.*)"};
                FileDialog chooseFiles = new FileDialog(getShell(), SWT.OPEN | SWT.MULTI);
                chooseFiles.setFilterExtensions(extensions);
                chooseFiles.setFilterNames(extNames);
                chooseFiles.open();
                String[] files = chooseFiles.getFileNames();
                for (int i = 0; i < files.length; ++i) {
                    String file = chooseFiles.getFilterPath() + files[i];
                    if (sourceList.indexOf(file) == -1) {
                        sourceList.add(file);
                    }
                }
                letContinue();
            }
            // Never called.
            public void widgetDefaultSelected(SelectionEvent e) {}
        });
        
        Button removeButton = new Button(this.sourceButtonsComposite, SWT.NONE);
        removeButton.setText("Remove");
        removeButton.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        removeButton.addSelectionListener(new SelectionListener() {
            public void widgetSelected(SelectionEvent e) {
                sourceList.remove(sourceList.getSelectionIndices());
                letContinue();
            }
            // Never called.
            public void widgetDefaultSelected(SelectionEvent e) {}
        });
    }

    /**
     * This function creates the whole area of questioning for Griffin's output
     * file.
     * 
     * @param parent The composite to put everything into.
     */
    private void outputFile(Composite parent) {
        // Create the container for the ouptut data.
        Composite composite = new Composite(parent, SWT.NONE);
        composite.setLayout(new GridLayout(1, false));
        composite.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        
        // Create the label in the output data (describe what's asked).
        Label description = new Label(composite, SWT.WRAP);
        description.setText("Please choose the location of Griffin's output file.");
        description.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        
        // Create the container for the file questioning field.
        Composite outputComposite = new Composite(composite, SWT.NONE);
        outputComposite.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        
        // Create the file questioning field
        this.outputFileField = new FileFieldEditor("outputFile","Output file:", outputComposite);
        String[] extensions = {"*.cc;*.cpp"};
        this.outputFileField.setFileExtensions(extensions);
        this.outputFileField.setEmptyStringAllowed(false);
        this.outputFileField.setStringValue("4robin.cc");
        this.outputFileField.getTextControl(outputComposite).addModifyListener(new ModifyListener() {
            public void modifyText(ModifyEvent e) {
                letContinue();
            }
        });
    }

    public boolean isDoxygen() {
        return this.doxygen;
    }

    public String getDoxygenXmlDir() {
        return this.doxygenXmlDirField.getStringValue();
    }

    public String getOutputFile() {
        return this.outputFileField.getStringValue();
    }

    public String[] getSourceList() {
        return this.sourceList.getItems();
    }
    
    private boolean doxygen = false;
    private Composite doxygenXmlDirComposite = null;
    private DirectoryFieldEditor doxygenXmlDirField = null;
    private List sourceList = null;
    private Composite sourceButtonsComposite = null;
    private FileFieldEditor outputFileField = null;
}

