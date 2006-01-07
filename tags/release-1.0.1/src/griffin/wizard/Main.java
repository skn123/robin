package wizard;
import org.eclipse.jface.wizard.WizardDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;

/*
 * Created on 12/10/2005 by Misha.
 */

public class Main {
    
    /**
     * @param args
     */
    public static void main(String[] args) {
        Display disp = new Display();
        Shell shell = new Shell(disp);
        
        WizardDialog wizard = new WizardDialog(shell, new GriffinWizard());
        wizard.open();
        
        disp.dispose();        
    }
}

