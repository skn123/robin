package wizard;
import java.io.IOException;
import java.util.logging.Level;

import org.eclipse.jface.wizard.Wizard;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;

import sourceanalysis.ElementNotFoundException;
import sourceanalysis.MissingInformationException;
import sourceanalysis.dox.DoxygenAnalyzer;
import backend.robin.Launcher;

public class GriffinWizard extends Wizard {
    public GriffinWizard() {
        this.setWindowTitle("Griffin");
        // Add the wizard pages.
        this.addPage(new FilesPage());
        this.addPage(new ClassesPage());
    }  

    @Override
	public boolean performFinish() {
        MessageBox box = new MessageBox(this.getShell(), SWT.ICON_ERROR | SWT.OK);
        box.setText("Error!");
        FilesPage filesPage = (FilesPage)this.getPage("Files");
        ClassesPage classesPage = (ClassesPage)this.getPage("Classes");
        String xmlDir = filesPage.getDoxygenXmlDir();
        if (!filesPage.isDoxygen()) {
            xmlDir = doDoxygen(filesPage.getSourceList());
        }
        if ((xmlDir == null) || (xmlDir.equals(""))) {
            return false;
        }
        DoxygenAnalyzer dox = new DoxygenAnalyzer(xmlDir);
        dox.logger.setLevel(Level.OFF);
        try {
            Launcher.execute(dox.processIndex(), classesPage.getClasses(), filesPage.getOutputFile(), false);
            return true;
        } catch (IOException e) {
            box.setMessage("Output error: " + e);
        } catch (MissingInformationException e) {
            box.setMessage("Some information is missing" + e);
        } catch (ElementNotFoundException e) {
            box.setMessage("Failed to read index: " + e);
        }
        box.open();
        return false;
    }
    
    private String doDoxygen(String[] sourceFiles) {
        MessageBox box = new MessageBox(this.getShell(), SWT.ICON_ERROR | SWT.OK);
        box.setText("Error!");
        box.setMessage("Running griffin straight on sources throw the wizard is not implemented yet.");
        box.open();
        return null;
    }
}

