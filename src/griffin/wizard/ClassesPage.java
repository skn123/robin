package wizard;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Text;
/*
 * Created on 12/10/2005 by Misha.
 */

public class ClassesPage extends WizardPage {
    public static final String DESCRIPTION_STR = "Choose what classes to wrap";
    
    public ClassesPage() {
        super("Classes");
        this.setTitle("Classes stage");
        this.setDescription(DESCRIPTION_STR);
    }

    public void createControl(Composite parent) {
        // Create the main composite.
        Composite composite = new Composite(parent, SWT.NONE);
        composite.setLayout(new GridLayout(1, false));
        composite.setLayoutData(new GridData(GridData.FILL_BOTH));
        
        // Make the label explaining the page.
        Label label = new Label(composite, SWT.WRAP);
        label.setText("Please write down the classes you want to wrap with whitespaces between them:");
        label.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
        
        // Add the text box, and check it's input for activation the finish button.
        this.classesBox(composite);
        this.classesBox.addModifyListener(new ModifyListener() {
            public void modifyText(ModifyEvent e) {
                if (classesBox.getText().equals("")) {
                    setPageComplete(false);
                    setMessage("You must specify at least one class or function to wrap.", ERROR);
                } else {
                    setPageComplete(true);
                    setMessage(DESCRIPTION_STR);
                }
            }
        });
        
        // Put the main composite on page.
        setControl(composite);
        
        // Deactiveate the finish button when not needed.
        composite.addListener(SWT.Show, new Listener() {
            public void handleEvent(Event event) {
                if (classesBox.getText().equals("")) {
                    setPageComplete(false);
                } else {
                    setPageComplete(true);
                }
            }
        });
        composite.addListener(SWT.Hide, new Listener() {
            public void handleEvent(Event event) {
                setPageComplete(false);
            }
        });
    }
    
    /**
     * This function creates the big text box for the classes to be written in.
     * 
     * @param parent The main composite to put the box into.
     */
    private void classesBox(Composite parent) {
        // Create a composite for the textbox.
        Composite composite = new Composite(parent, SWT.NONE);
        composite.setLayout(new GridLayout(1, false));
        composite.setLayoutData(new GridData(GridData.FILL_BOTH));
        
        // Define the textbox itself.
        this.classesBox = new Text(composite, SWT.MULTI | SWT.WRAP | SWT.BORDER | SWT.V_SCROLL);
        this.classesBox.setLayoutData(new GridData(GridData.FILL_BOTH));
    }
    
    public String[] getClasses() {
        return classesBox.getText().split("\\s+");
    }
    
    Text classesBox = null;
}

