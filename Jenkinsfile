import groovy.json.JsonSlurper
/*
** Variables.
*/
properties([buildDiscarder(logRotator(numToKeepStr: '50'))])
def serie = '21.10'
def maintenanceBranch = "${serie}.x"
if (env.BRANCH_NAME.startsWith('release-')) {
  env.BUILD = 'RELEASE'
} else if ((env.BRANCH_NAME == 'master') || (env.BRANCH_NAME == maintenanceBranch)) {
  env.BUILD = 'REFERENCE'
} else {
  env.BUILD = 'CI'
}

/*
** Pipeline code.
*/
stage('Source') {
  node {
    sh 'setup_centreon_build.sh'
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
  stage('Unit tests') {
    parallel 'centos7': {
      node {
        sh 'setup_centreon_build.sh'
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
    'centos8': {
      node {
        sh 'setup_centreon_build.sh'
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-unittest.sh centos8"
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
    'debian10': {
      node {
        sh 'setup_centreon_build.sh'
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
    }
    if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
      error('Unit tests stage failure.');
    }
  }

  // sonarQube step to get qualityGate result
  stage('Quality gate') {
    node {
      def reportFilePath = "target/sonar/report-task.txt"
      def reportTaskFileExists = fileExists "${reportFilePath}"
      if (reportTaskFileExists) {
        echo "Found report task file"
        def taskProps = readProperties file: "${reportFilePath}"
        echo "taskId[${taskProps['ceTaskId']}]"
        timeout(time: 10, unit: 'MINUTES') {
          while (true) {
            sleep 5
            def taskStatusResult    =
              sh(returnStdout: true, script: "curl -s -X GET -u ${authString} \'${sonarProps['sonar.host.url']}/api/ce/task?id=${taskProps['ceTaskId']}\'")
              echo "taskStatusResult[${taskStatusResult}]"
              def taskStatus  = new JsonSlurper().parseText(taskStatusResult).task.status
              echo "taskStatus[${taskStatus}]"
              // Status can be SUCCESS, ERROR, PENDING, or IN_PROGRESS. The last two indicate it's
              // not done yet.
              if (taskStatus != "IN_PROGRESS" && taskStatus != "PENDING") {
                  break;
              }
              def qualityGate = waitForQualityGate()
              if (qualityGate.status != 'OK') {
                currentBuild.result = 'FAIL'
              }
          }
        }
      }
      if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
        error('Quality gate failure: ${qualityGate.status}.');
      }
    }
  }

  stage('Package') {
    parallel 'centos7': {
      node {
        sh 'setup_centreon_build.sh'
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-package.sh centos7"
      }
    },
    'centos8': {
      node {
        sh 'setup_centreon_build.sh'
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-package.sh centos8"
      }
    },
    'debian10': {
      node {
        sh 'setup_centreon_build.sh'
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-package.sh debian10"
      }
/*
    },
    'debian10-armhf': {
      node {
        sh 'setup_centreon_build.sh'
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-package.sh debian10-armhf"
      }
*/
    }
    if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
      error('Package stage failure.');
    }
  }

  if ((env.BUILD == 'RELEASE') || (env.BUILD == 'REFERENCE')) {
    stage('Delivery') {
      node {
        sh 'setup_centreon_build.sh'
        sh "./centreon-build/jobs/engine/${serie}/mon-engine-delivery.sh"
      }
      if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
        error('Delivery stage failure.');
      }
    }

    if (env.BUILD == 'REFERENCE') {
      build job: "centreon-web/${env.BRANCH_NAME}", wait: false
    }
  }
}
finally {
  buildStatus = currentBuild.result ?: 'SUCCESS';
  if ((buildStatus != 'SUCCESS') && ((env.BUILD == 'RELEASE') || (env.BUILD == 'REFERENCE'))) {
    slackSend channel: '#monitoring-metrology', message: "@channel Centreon Engine build ${env.BUILD_NUMBER} of branch ${env.BRANCH_NAME} was broken by ${source.COMMITTER}. Please fix it ASAP."
  }
}
