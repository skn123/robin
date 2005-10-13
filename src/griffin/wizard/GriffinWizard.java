package wizard;
import org.eclipse.jface.wizard.Wizard;
/*
 * Created on 12/10/2005 by Misha.
 */

public class GriffinWizard extends Wizard {
    public GriffinWizard() {
        this.setWindowTitle("Griffin");
        // Add the wizard pages.
        this.addPage(new FilesPage());
        this.addPage(new ClassesPage());
    }  

    @Override
    public boolean performFinish() {
        // TODO Auto-generated method stub
        System.out.println("FINISH!");
        return true;
    }
}

