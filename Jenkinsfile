/*
    fty-core - Low-level scripts for 42ITy based product bundle

    Copyright (C) 2014 - 2020 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    NOTE : This Jenkins pipeline script only handles the self-testing of your
    project. If you also want the successful codebase published or deployed,
    you can define a helper job - see the reference implementation skeleton at
    https://github.com/zeromq/zproject/blob/master/Jenkinsfile-deploy.example

*/

pipeline {
    agent { label "devel-image && x86_64" }
    parameters {
        // Use DEFAULT_DEPLOY_BRANCH_PATTERN and DEFAULT_DEPLOY_JOB_NAME if
        // defined in this jenkins setup -- in Jenkins Management Web-GUI
        // see Configure System / Global properties / Environment variables
        // Default (if unset) is empty => no deployment attempt after good test
        // See zproject Jenkinsfile-deploy.example for an example deploy job.
        // TODO: Try to marry MultiBranchPipeline support with pre-set defaults
        // directly in MultiBranchPipeline plugin, or mechanism like Credentials,
        // or a config file uploaded to master for all jobs or this job, see
        // https://jenkins.io/doc/pipeline/examples/#configfile-provider-plugin
        string (
            defaultValue: '${DEFAULT_DEPLOY_BRANCH_PATTERN}',
            description: 'Regular expression of branch names for which a deploy action would be attempted after a successful build and test; leave empty to not deploy. Reasonable value is ^(master|release/.*|feature/*)$',
            name : 'DEPLOY_BRANCH_PATTERN')
        string (
            defaultValue: '${DEFAULT_DEPLOY_JOB_NAME}',
            description: 'Name of your job that handles deployments and should accept arguments: DEPLOY_GIT_URL DEPLOY_GIT_BRANCH DEPLOY_GIT_COMMIT -- and it is up to that job what to do with this knowledge (e.g. git archive + push to packaging); leave empty to not deploy',
            name : 'DEPLOY_JOB_NAME')
        booleanParam (
            defaultValue: true,
            description: 'If the deployment is done, should THIS job wait for it to complete and include its success or failure as the build result (true), or should it schedule the job and exit quickly to free up the executor (false)',
            name: 'DEPLOY_REPORT_RESULT')
        booleanParam (
            defaultValue: false,
            description: 'Attempt "make check" in this run?',
            name: 'DO_TEST_CHECK')
        booleanParam (
            defaultValue: false,
            description: 'Attempt "make memcheck" in this run?',
            name: 'DO_TEST_MEMCHECK')
        booleanParam (
            defaultValue: true,
            description: 'Attempt "make distcheck" in this run?',
            name: 'DO_TEST_DISTCHECK')
        booleanParam (
            defaultValue: false,
            description: 'Attempt a "make install" check in this run?',
            name: 'DO_TEST_INSTALL')
        string (
            defaultValue: "`pwd`/tmp/_inst",
            description: 'If attempting a "make install" check in this run, what DESTDIR to specify? (absolute path, defaults to "BUILD_DIR/tmp/_inst")',
            name: 'USE_TEST_INSTALL_DESTDIR')
        booleanParam (
            defaultValue: false,
            description: 'Attempt "cppcheck" analysis before this run? (Note: corresponding tools are required in the build environment)',
            name: 'DO_CPPCHECK')
        booleanParam (
            defaultValue: true,
            description: 'Require that there are no files not discovered changed/untracked via .gitignore after builds and tests?',
            name: 'CI_REQUIRE_GOOD_GITIGNORE')
        string (
            defaultValue: "20",
            description: 'When running tests, use this timeout (in minutes; be sure to leave enough for double-job of a distcheck too)',
            name: 'USE_TEST_TIMEOUT')
        booleanParam (
            defaultValue: true,
            description: 'When using temporary subdirs in build/test workspaces, wipe them right after each successful build stage?',
            name: 'DO_CLEANUP_AFTER_BUILD')
        booleanParam (
            defaultValue: true,
            description: 'When using temporary subdirs in build/test workspaces, wipe them after the whole job is done successfully?',
            name: 'DO_CLEANUP_AFTER_JOB')
        booleanParam (
            defaultValue: true,
            description: 'When using temporary subdirs in build/test workspaces, wipe them after the whole job is done unsuccessfully (failed)? Note this would not allow postmortems on CI server, but would conserve its disk space.',
            name: 'DO_CLEANUP_AFTER_FAILED_JOB')
    }
    triggers {
        pollSCM 'H/2 * * * *'
    }

    // Jenkins tends to reschedule jobs that have not yet completed if they took
    // too long, maybe this happens in combination with polling. Either way, if
    // the server gets into this situation, the snowball of same builds grows as
    // the build system lags more and more. Let the devs avoid it in a few ways.
    options {
        disableConcurrentBuilds()
        // Jenkins community suggested that instead of a default checkout, we can do
        // an explicit step for that. It is expected that either way Jenkins "should"
        // record that a particular commit is being processed, but the explicit ways
        // might work better. In either case it honors SCM settings like refrepo if
        // set up in the Pipeline or MultiBranchPipeline job.
        skipDefaultCheckout()
    }
// Note: your Jenkins setup may benefit from similar setup on side of agents:
//        PATH="/usr/lib64/ccache:/usr/lib/ccache:/usr/bin:/bin:${PATH}"
    stages {
        stage ('pre-clean') {
                    steps {
                        milestone ordinal: 20, label: "${env.JOB_NAME}@${env.BRANCH_NAME}"
                        dir("tmp") {
                            sh 'if [ -s Makefile ]; then make -k distclean || true ; fi'
                            sh 'chmod -R u+w .'
                            deleteDir()
                        }
                        sh 'rm -f ccache.log cppcheck.xml'
                    }
        }
        stage ('git') {
                    steps {
                        retry(3) {
                            checkout scm
                        }
                        milestone ordinal: 30, label: "${env.JOB_NAME}@${env.BRANCH_NAME}"
                    }
        }
        stage ('prepare') {
                    steps {
                        sh './autogen.sh'
                        stash (name: 'prepped', includes: '**/*', excludes: '**/cppcheck.xml')
                        milestone ordinal: 40, label: "${env.JOB_NAME}@${env.BRANCH_NAME}"
                    }
        }
        stage ('configure') {
                    steps {
                        sh 'CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; ./configure'
                    }
        }
        stage ('compile') {
                    steps {
                        sh 'CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; make -k -j4 all || make all'
                        sh 'echo "Are GitIgnores good after make? (should have no output below)"; git status -s || if [ "${params.CI_REQUIRE_GOOD_GITIGNORE}" = false ]; then echo "WARNING GitIgnore tests found newly changed or untracked files" >&2 ; exit 0 ; else echo "FAILED GitIgnore tests" >&2 ; exit 1; fi'
                        script {
                          if ( (params.DO_TEST_CHECK && params.DO_TEST_MEMCHECK) || (params.DO_TEST_CHECK && params.DO_TEST_DISTCHECK) || (params.DO_TEST_MEMCHECK && params.DO_TEST_DISTCHECK) ||
                               (params.DO_TEST_INSTALL && params.DO_TEST_MEMCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_DISTCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_CHECK)
                             ) {
                                stash (name: 'built', includes: '**/*')
                            }
                        }
                    }
        }
        stage ('check') {
            parallel {
                stage ('cppcheck') {
                    when { expression { return ( params.DO_CPPCHECK ) } }
                    steps {
                        dir("tmp/test-cppcheck") {
                            deleteDir()
                            unstash 'prepped'
                            sh 'cppcheck --std=c++11 --enable=all --inconclusive --xml --xml-version=2 . 2>cppcheck.xml'
                            archiveArtifacts artifacts: '**/cppcheck.xml'
                            sh 'rm -f cppcheck.xml'
                            script {
                                if ( params.DO_CLEANUP_AFTER_BUILD ) {
                                    deleteDir()
                                }
                            }
                        }
                    }
                }
                stage ('make check') {
                    when { expression { return ( params.DO_TEST_CHECK ) } }
                    steps {
                      script {
                          if ( (params.DO_TEST_CHECK && params.DO_TEST_MEMCHECK) || (params.DO_TEST_CHECK && params.DO_TEST_DISTCHECK) || (params.DO_TEST_MEMCHECK && params.DO_TEST_DISTCHECK) ||
                               (params.DO_TEST_INSTALL && params.DO_TEST_MEMCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_DISTCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_CHECK)
                             ) {
                            dir("tmp/test-check") {
                                deleteDir()
                                unstash 'built'
                                timeout (time: "${params.USE_TEST_TIMEOUT}".toInteger(), unit: 'MINUTES') {
                                    sh 'CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; make check'
                                }
                                sh 'echo "Are GitIgnores good after make check? (should have no output below)"; git status -s || if [ "${params.CI_REQUIRE_GOOD_GITIGNORE}" = false ]; then echo "WARNING GitIgnore tests found newly changed or untracked files" >&2 ; exit 0 ; else echo "FAILED GitIgnore tests" >&2 ; exit 1; fi'
                                script {
                                    if ( params.DO_CLEANUP_AFTER_BUILD ) {
                                        deleteDir()
                                    }
                                }
                            }
                          } else {
                                timeout (time: "${params.USE_TEST_TIMEOUT}".toInteger(), unit: 'MINUTES') {
                                    sh 'CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; make check'
                                }
                                sh 'echo "Are GitIgnores good after make check? (should have no output below)"; git status -s || if [ "${params.CI_REQUIRE_GOOD_GITIGNORE}" = false ]; then echo "WARNING GitIgnore tests found newly changed or untracked files" >&2 ; exit 0 ; else echo "FAILED GitIgnore tests" >&2 ; exit 1; fi'
                          }
                        }
                    }
                }
                stage ('make memcheck') {
                    when { expression { return ( params.DO_TEST_MEMCHECK ) } }
                    steps {
                        script {
                          if ( (params.DO_TEST_CHECK && params.DO_TEST_MEMCHECK) || (params.DO_TEST_CHECK && params.DO_TEST_DISTCHECK) || (params.DO_TEST_MEMCHECK && params.DO_TEST_DISTCHECK) ||
                               (params.DO_TEST_INSTALL && params.DO_TEST_MEMCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_DISTCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_CHECK)
                             ) {
                              dir("tmp/test-memcheck") {
                                deleteDir()
                                unstash 'built'
                                timeout (time: "${params.USE_TEST_TIMEOUT}".toInteger(), unit: 'MINUTES') {
                                    sh 'CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; make memcheck && exit 0 ; echo "Re-running failed ($?) memcheck with greater verbosity" >&2 ; make VERBOSE=1 memcheck-verbose'
                                }
                                sh 'echo "Are GitIgnores good after make memcheck? (should have no output below)"; git status -s || if [ "${params.CI_REQUIRE_GOOD_GITIGNORE}" = false ]; then echo "WARNING GitIgnore tests found newly changed or untracked files" >&2 ; exit 0 ; else echo "FAILED GitIgnore tests" >&2 ; exit 1; fi'
                                script {
                                    if ( params.DO_CLEANUP_AFTER_BUILD ) {
                                        deleteDir()
                                    }
                                }
                              }
                          } else {
                                timeout (time: "${params.USE_TEST_TIMEOUT}".toInteger(), unit: 'MINUTES') {
                                    sh 'CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; make memcheck && exit 0 ; echo "Re-running failed ($?) memcheck with greater verbosity" >&2 ; make VERBOSE=1 memcheck-verbose'
                                }
                                sh 'echo "Are GitIgnores good after make memcheck? (should have no output below)"; git status -s || if [ "${params.CI_REQUIRE_GOOD_GITIGNORE}" = false ]; then echo "WARNING GitIgnore tests found newly changed or untracked files" >&2 ; exit 0 ; else echo "FAILED GitIgnore tests" >&2 ; exit 1; fi'
                          }
                      }
                    }
                }
                stage ('make distcheck') {
                    when { expression { return ( params.DO_TEST_DISTCHECK ) } }
                    steps {
                        script {
                          if ( (params.DO_TEST_CHECK && params.DO_TEST_MEMCHECK) || (params.DO_TEST_CHECK && params.DO_TEST_DISTCHECK) || (params.DO_TEST_MEMCHECK && params.DO_TEST_DISTCHECK) ||
                               (params.DO_TEST_INSTALL && params.DO_TEST_MEMCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_DISTCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_CHECK)
                             ) {
                              dir("tmp/test-distcheck") {
                                deleteDir()
                                unstash 'built'
                                timeout (time: "${params.USE_TEST_TIMEOUT}".toInteger(), unit: 'MINUTES') {
                                    sh 'CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; make distcheck'
                                }
                                sh 'echo "Are GitIgnores good after make distcheck? (should have no output below)"; git status -s || if [ "${params.CI_REQUIRE_GOOD_GITIGNORE}" = false ]; then echo "WARNING GitIgnore tests found newly changed or untracked files" >&2 ; exit 0 ; else echo "FAILED GitIgnore tests" >&2 ; exit 1; fi'
                                script {
                                    if ( params.DO_CLEANUP_AFTER_BUILD ) {
                                        deleteDir()
                                    }
                                }
                              }
                            } else {
                                timeout (time: "${params.USE_TEST_TIMEOUT}".toInteger(), unit: 'MINUTES') {
                                    sh 'CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; make distcheck'
                                }
                                sh 'echo "Are GitIgnores good after make distcheck? (should have no output below)"; git status -s || if [ "${params.CI_REQUIRE_GOOD_GITIGNORE}" = false ]; then echo "WARNING GitIgnore tests found newly changed or untracked files" >&2 ; exit 0 ; else echo "FAILED GitIgnore tests" >&2 ; exit 1; fi'
                            }
                        }
                    }
                }
                stage ('make install check') {
                    when { expression { return ( params.DO_TEST_INSTALL ) } }
                    steps {
                        script {
                          if ( (params.DO_TEST_CHECK && params.DO_TEST_MEMCHECK) || (params.DO_TEST_CHECK && params.DO_TEST_DISTCHECK) || (params.DO_TEST_MEMCHECK && params.DO_TEST_DISTCHECK) ||
                               (params.DO_TEST_INSTALL && params.DO_TEST_MEMCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_DISTCHECK) || (params.DO_TEST_INSTALL && params.DO_TEST_CHECK)
                             ) {
                              dir("tmp/test-install-check") {
                                deleteDir()
                                unstash 'built'
                                timeout (time: "${params.USE_TEST_TIMEOUT}".toInteger(), unit: 'MINUTES') {
                                    sh """CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; make DESTDIR="${params.USE_TEST_INSTALL_DESTDIR}" install"""
                                }
                                sh 'echo "Are GitIgnores good after make install? (should have no output below)"; git status -s || if [ "${params.CI_REQUIRE_GOOD_GITIGNORE}" = false ]; then echo "WARNING GitIgnore tests found newly changed or untracked files" >&2 ; exit 0 ; else echo "FAILED GitIgnore tests" >&2 ; exit 1; fi'
                                script {
                                    if ( params.DO_CLEANUP_AFTER_BUILD ) {
                                        deleteDir()
                                    }
                                }
                              }
                            } else {
                                timeout (time: "${params.USE_TEST_TIMEOUT}".toInteger(), unit: 'MINUTES') {
                                    sh """CCACHE_BASEDIR="`pwd`" ; export CCACHE_BASEDIR; make DESTDIR="${params.USE_TEST_INSTALL_DESTDIR}" install"""
                                }
                                sh 'echo "Are GitIgnores good after make install? (should have no output below)"; git status -s || if [ "${params.CI_REQUIRE_GOOD_GITIGNORE}" = false ]; then echo "WARNING GitIgnore tests found newly changed or untracked files" >&2 ; exit 0 ; else echo "FAILED GitIgnore tests" >&2 ; exit 1; fi'
                            }
                        }
                    }
                }
            }
        }
        stage ('deploy if appropriate') {
            steps {
                script {
                    def myDEPLOY_JOB_NAME = sh(returnStdout: true, script: """echo "${params["DEPLOY_JOB_NAME"]}" """).trim();
                    def myDEPLOY_BRANCH_PATTERN = sh(returnStdout: true, script: """echo "${params["DEPLOY_BRANCH_PATTERN"]}" """).trim();
                    def myDEPLOY_REPORT_RESULT = sh(returnStdout: true, script: """echo "${params["DEPLOY_REPORT_RESULT"]}" """).trim().toBoolean();
                    echo "Original: DEPLOY_JOB_NAME : ${params["DEPLOY_JOB_NAME"]} DEPLOY_BRANCH_PATTERN : ${params["DEPLOY_BRANCH_PATTERN"]} DEPLOY_REPORT_RESULT : ${params["DEPLOY_REPORT_RESULT"]}"
                    echo "Used:     myDEPLOY_JOB_NAME:${myDEPLOY_JOB_NAME} myDEPLOY_BRANCH_PATTERN:${myDEPLOY_BRANCH_PATTERN} myDEPLOY_REPORT_RESULT:${myDEPLOY_REPORT_RESULT}"
                    if ( (myDEPLOY_JOB_NAME != "") && (myDEPLOY_BRANCH_PATTERN != "") ) {
                        if ( env.BRANCH_NAME =~ myDEPLOY_BRANCH_PATTERN ) {
                            def GIT_URL = sh(returnStdout: true, script: """git remote -v | egrep '^origin' | awk '{print \$2}' | head -1""").trim()
                            def GIT_COMMIT = sh(returnStdout: true, script: 'git rev-parse --verify HEAD').trim()
                            def DIST_ARCHIVE = ""
                            //if ( params.DO_DIST_DOCS ) { DIST_ARCHIVE = env.BUILD_URL + "artifact/__dist.tar.gz" }
                            echo "Would deploy ${GIT_URL} ${GIT_COMMIT} because tested branch '${env.BRANCH_NAME}' matches filter '${myDEPLOY_BRANCH_PATTERN}'"
                            milestone ordinal: 100, label: "${env.JOB_NAME}@${env.BRANCH_NAME}"
                            build job: "${myDEPLOY_JOB_NAME}", parameters: [
                                string(name: 'DEPLOY_GIT_URL', value: "${GIT_URL}"),
                                string(name: 'DEPLOY_GIT_BRANCH', value: env.BRANCH_NAME),
                                string(name: 'DEPLOY_GIT_COMMIT', value: "${GIT_COMMIT}"),
                                string(name: 'DEPLOY_DIST_ARCHIVE', value: "${DIST_ARCHIVE}")
                                ], quietPeriod: 0, wait: myDEPLOY_REPORT_RESULT, propagate: myDEPLOY_REPORT_RESULT
                        } else {
                            echo "Not deploying because branch '${env.BRANCH_NAME}' did not match filter '${myDEPLOY_BRANCH_PATTERN}'"
                        }
                    } else {
                        echo "Not deploying because deploy-job parameters are not set"
                    }
                }
            }
        }
        stage ('cleanup') {
            when { expression { return ( params.DO_CLEANUP_AFTER_BUILD ) } }
            steps {
                deleteDir()
            }
        }
    }
    post {
        success {
            script {
                if (currentBuild.getPreviousBuild()?.result != 'SUCCESS') {
                    // Uncomment desired notification

                    //slackSend (color: "#008800", message: "Build ${env.JOB_NAME} is back to normal.")
                    //emailext (to: "qa@example.com", subject: "Build ${env.JOB_NAME} is back to normal.", body: "Build ${env.JOB_NAME} is back to normal.")
                }
                if ( params.DO_CLEANUP_AFTER_JOB ) {
                    dir("tmp") {
                        deleteDir()
                    }
                }
            }
        }
        failure {
            // Uncomment desired notification
            // Section must not be empty, you can delete the sleep once you set notification
            sleep 1
            //slackSend (color: "#AA0000", message: "Build ${env.BUILD_NUMBER} of ${env.JOB_NAME} ${currentBuild.result} (<${env.BUILD_URL}|Open>)")
            //emailext (to: "qa@example.com", subject: "Build ${env.JOB_NAME} failed!", body: "Build ${env.BUILD_NUMBER} of ${env.JOB_NAME} ${currentBuild.result}\nSee ${env.BUILD_URL}")

            dir("tmp") {
                script {
                    if ( params.DO_CLEANUP_AFTER_FAILED_JOB ) {
                        deleteDir()
                    } else {
                        sh """ echo "NOTE: BUILD AREA OF WORKSPACE `pwd` REMAINS FOR POST-MORTEMS ON `hostname` AND CONSUMES `du -hs . | awk '{print \$1}'` !" """
                    }
                }
            }
        }
    }
}
