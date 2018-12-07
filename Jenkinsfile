stage('Source') {
  node {
    sh 'setup_centreon_build.sh'
    dir('centreon-engine') {
      checkout scm
    }
    sh './centreon-build/jobs/engine/3.4/mon-engine-source.sh'
    source = readProperties file: 'source.properties'
    env.VERSION = "${source.VERSION}"
    env.RELEASE = "${source.RELEASE}"
  }
}

try {
  stage('Unit tests') {
    parallel 'centos6': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/3.4/mon-engine-unittest.sh centos6'
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
    'centos7': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/3.4/mon-engine-unittest.sh centos7'
        step([
          $class: 'XUnitBuilder',
          thresholds: [
            [$class: 'FailedThreshold', failureThreshold: '0'],
            [$class: 'SkippedThreshold', failureThreshold: '0']
          ],
          tools: [[$class: 'GoogleTestType', pattern: 'ut.xml']]
        ])
        if (env.BRANCH_NAME == 'master') {
          withSonarQubeEnv('SonarQube') {
            sh './centreon-build/jobs/engine/3.4/mon-engine-analysis.sh'
          }
        }
      }
    }
    if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
      error('Unit tests stage failure.');
    }
  }

  stage('Package') {
    parallel 'centos6': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/3.4/mon-engine-package.sh centos6'
      }
    },
    'centos7': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/3.4/mon-engine-package.sh centos7'
      }
    }
    if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
      error('Package stage failure.');
    }
  }

  if (env.BRANCH_NAME == '1.8.x') {
    build job: 'centreon-web/2.8.x', wait: false
  }
}
finally {
  buildStatus = currentBuild.result ?: 'SUCCESS';
  if ((buildStatus != 'SUCCESS') && (env.BRANCH_NAME == '1.8.x')) {
    slackSend channel: '#monitoring-metrology', message: "@channel Centreon Engine build ${env.BUILD_NUMBER} of branch ${env.BRANCH_NAME} was broken by ${source.COMMITTER}. Please fix it ASAP."
  }
}
