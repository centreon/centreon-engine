/*
** Variables.
*/
def serie = '21.04'
def maintenanceBranch = "${serie}.x"
def qaBranch = "dev-${serie}.x"
if (env.BRANCH_NAME.startsWith('release-')) {
  env.BUILD = 'RELEASE'
} else if ((env.BRANCH_NAME == 'master') || (env.BRANCH_NAME == maintenanceBranch)) {
  env.BUILD = 'REFERENCE'
} else if ((env.BRANCH_NAME == 'develop') || (env.BRANCH_NAME == qaBranch)) {
  env.BUILD = 'QA'
} else {
  env.BUILD = 'CI'
}

def checkoutCentreonBuild() {
  dir('centreon-build') {
    checkout resolveScm(source: [$class: 'GitSCMSource',
      remote: 'https://github.com/centreon/centreon-build.git',
      credentialsId: 'technique-ci',
      traits: [[$class: 'jenkins.plugins.git.traits.BranchDiscoveryTrait']]],
      targets: [BRANCH_NAME, 'master'])
  }
}

/*
** Pipeline code.
*/
stage('Source') {
  node("C++") {
    checkoutCentreonBuild()
    dir('centreon-engine') {
      checkout scm
    }
    sh "./centreon-build/jobs/engine/${serie}/mon-engine-source.sh"
    source = readProperties file: 'source.properties'
    env.VERSION = "${source.VERSION}"
    env.RELEASE = "${source.RELEASE}"
    publishHTML([
      allowMissing: false,
      keepAll: true,
      reportDir: 'summary',
      reportFiles: 'index.html',
      reportName: 'Centreon Engine Build Artifacts',
      reportTitles: ''
    ])
  }
}

try {
  stage('Build // Unit Tests // RPMs Packaging') {
    parallel 'build centos7': {
      node("C++") {
        checkoutCentreonBuild()
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-unittest.sh centos7"
        step([
          $class: 'XUnitBuilder',
          thresholds: [
            [$class: 'FailedThreshold', failureThreshold: '0'],
            [$class: 'SkippedThreshold', failureThreshold: '0']
          ],
          tools: [[$class: 'GoogleTestType', pattern: 'ut.xml']]
        ])
        withSonarQubeEnv('SonarQubeDev') {
          sh "./centreon-build/jobs/engine/${serie}/mon-engine-analysis.sh"
        }
      }
    },
    'packaging centos7': {
      node("C++") {
        checkoutCentreonBuild()
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-package.sh centos7"
        stash name: 'el7-rpms', includes: "output/x86_64/*.rpm"
        archiveArtifacts artifacts: "output/x86_64/*.rpm"
      }
    },
    'build alma8': {
      node("C++") {
        checkoutCentreonBuild()
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-unittest.sh alma8"
        step([
          $class: 'XUnitBuilder',
          thresholds: [
            [$class: 'FailedThreshold', failureThreshold: '0'],
            [$class: 'SkippedThreshold', failureThreshold: '0']
          ],
          tools: [[$class: 'GoogleTestType', pattern: 'ut.xml']]
        ])
      }
    },
    'packaging alma8': {
      node("C++") {
        checkoutCentreonBuild()
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-package.sh alma8"
        stash name: 'alma8-rpms', includes: "output/x86_64/*.rpm"
        archiveArtifacts artifacts: "output/x86_64/*.rpm"
      }
    },
    'build debian10': {
      node("C++") {
        checkoutCentreonBuild()
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-unittest.sh debian10"
        step([
          $class: 'XUnitBuilder',
          thresholds: [
            [$class: 'FailedThreshold', failureThreshold: '0'],
            [$class: 'SkippedThreshold', failureThreshold: '0']
          ],
          tools: [[$class: 'GoogleTestType', pattern: 'ut.xml']]
        ])
      }
    },
    'packaging debian10': {
      node("C++") {
        checkoutCentreonBuild()
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-package.sh debian10"
      }
    }
    if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
      error('Unit tests stage failure.');
    }
  }

  // sonarQube step to get qualityGate result
  stage('Quality gate') {
    node("C++") {
      timeout(time: 10, unit: 'MINUTES') {
        def qualityGate = waitForQualityGate()
        if (qualityGate.status != 'OK') {
          currentBuild.result = 'FAIL'
        }
      }
      if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
        error('Quality gate failure: ${qualityGate.status}.');
      }
    }
  }

  if ((env.BUILD == 'RELEASE') || (env.BUILD == 'QA')) {
    stage('Delivery') {
      node {
        unstash 'el7-rpms'
        unstash 'alma8-rpms'
        checkoutCentreonBuild()
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-delivery.sh"
      }
      if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
        error('Delivery stage failure.');
      }
    }
  }
  if (env.BUILD == 'REFERENCE') {
      build job: "centreon-web/${env.BRANCH_NAME}", wait: false
  }
}
finally {
  buildStatus = currentBuild.result ?: 'SUCCESS';
  if ((buildStatus != 'SUCCESS') && ((env.BUILD == 'RELEASE') || (env.BUILD == 'REFERENCE'))) {
    slackSend channel: '#monitoring-metrology', message: "@channel Centreon Engine build ${env.BUILD_NUMBER} of branch ${env.BRANCH_NAME} was broken by ${source.COMMITTER}. Please fix it ASAP."
  }
}
