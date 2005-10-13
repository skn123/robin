package wizard;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
/*
 * Created on 12/10/2005 by Misha.
 */

public class ClassesPage extends WizardPage {
    public ClassesPage() {
        super("Classes");
        this.setTitle("Classes stage");
        this.setDescription("Choose what classes to wrap");
        this.setPageComplete(false);
    }

    public void createControl(Composite parent) {
        Label label = new Label(parent, SWT.CENTER);
        label.setText("Classes Page");
        setControl(label);
    }

}

