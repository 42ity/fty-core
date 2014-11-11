import hudson.model.*
import hudson.utils.*



def startJob(name,params) {
    job = Hudson.instance.getJob(name);
    job.scheduleBuild2(0, new Cause.UpstreamCause(build), new ParametersAction(params) );
    sleep(1000);
}

def waitForJob(name) {
    jobs = Hudson.instance.getJob(name);
    while( job.isInQueue() || job.isBuilding() ) {
        sleep(1000);
    }
}

// Start another jobs to reset machines
for( hypervisor in ["mirabox.roz.lab.etn.com", "debian.roz.lab.etn.com"]) {
    println "reseting on $hypervisor";
    startJob(
        "reset_virtual_machine",
        [ new StringParameterValue('HYPERVISOR', hypervisor), ]
    );
    waitForJob("reset_virtual_machine");
}
