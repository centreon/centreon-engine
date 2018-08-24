stage('Source') {
  node {
    sh 'setup_centreon_build.sh'
    dir('centreon-engine') {
      checkout scm
    }
    sh './centreon-build/jobs/engine/18.9/mon-engine-source.sh'
    source = readProperties file: 'source.properties'
    env.VERSION = "${source.VERSION}"
    env.RELEASE = "${source.RELEASE}"
  }
}

try {
  stage('Unit tests') {
    parallel 'centos7': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/18.9/mon-engine-unittest.sh centos7'
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
    'debian9': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/18.9/mon-engine-unittest.sh debian9'
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
        sh './centreon-build/jobs/engine/18.9/mon-engine-unittest.sh debian10'
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

  stage('Package') {
    parallel 'centos7': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/18.9/mon-engine-package.sh centos7'
      }
    },
    'debian9': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/18.9/mon-engine-package.sh debian9'
      }
    },
    'debian9-armhf': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/18.9/mon-engine-package.sh debian9-armhf'
      }
    },
    'debian10': {
      node {
        sh 'setup_centreon_build.sh'
        sh './centreon-build/jobs/engine/18.9/mon-engine-package.sh debian10'
      }
    }
    if ((currentBuild.result ?: 'SUCCESS') != 'SUCCESS') {
      error('Package stage failure.');
    }
  }

  if (env.BRANCH_NAME == 'master') {
    build job: 'centreon-web/master', wait: false
  }
}
finally {
  buildStatus = currentBuild.result ?: 'SUCCESS';
  if ((buildStatus != 'SUCCESS') && (env.BRANCH_NAME == 'master')) {
    slackSend channel: '#monitoring-metrology', message: "@channel Centreon Engine build ${env.BUILD_NUMBER} of branch ${env.BRANCH_NAME} was broken by ${source.COMMITTER}. Please fix it ASAP."
  }
}
