@Library("centreon-shared-library")_

/*
** Variables.
*/

properties([buildDiscarder(logRotator(numToKeepStr: '10'))])
env.REF_BRANCH = 'master'
env.PROJECT='centreon-engine'
def serie = '21.10'
def maintenanceBranch = "${serie}.x"
def qaBranch = "dev-${serie}.x"
def buildBranch = env.BRANCH_NAME
if (env.CHANGE_BRANCH) {
  buildBranch = env.CHANGE_BRANCH
}

/*
** Branch management
*/
if (env.BRANCH_NAME.startsWith('release-')) {
  env.BUILD = 'RELEASE'
} else if ((env.BRANCH_NAME == env.REF_BRANCH) || (env.BRANCH_NAME == maintenanceBranch)) {
  env.BUILD = 'REFERENCE'
} else if ((env.BRANCH_NAME == 'develop') || (env.BRANCH_NAME == qaBranch)) {
  env.BUILD = 'QA'
} else {
  env.BUILD = 'CI'
}

/*
** Pipeline code.
*/
stage('Deliver sources') {
  node("C++") {
    dir('centreon-engine-centos7') {
      checkout scm
      loadCommonScripts()
      /* change to engine */
      
      sh 'ci/scripts/engine-sources-delivery.sh centreon-engine'
      source = readProperties file: 'source.properties'
      env.VERSION = "${source.VERSION}"
      env.RELEASE = "${source.RELEASE}"
    }
  }
}

stage('Build / Unit tests // Packaging / Signing') {
  parallel 'centos7 Build and UT': {
    node("C++") {
      dir('centreon-engine-centos7') {
        checkout scm
        sh 'docker run -i --entrypoint /src/ci/scripts/engine-unit-tests.sh -v "$PWD:/src" registry.centreon.com/centreon-collect-centos7-dependencies:21.10'
        sh "sudo apt-get install -y clang-tidy"
        withSonarQubeEnv('SonarQubeDev') {
          sh 'ci/scripts/engine-sources-analysis.sh'
        }
      }
    }
  },
  'centos7 rpm packaging and signing': {
    node("C++") {
      dir('centreon-engine-centos7') {
        checkout scm
        sh 'docker run -i --entrypoint /src/centreon-engine/ci/scripts/engine-rpm-package.sh -v "$PWD:/src/centreon-engine" -e DISTRIB="el7" -e VERSION=$VERSION -e RELEASE=$RELEASE registry.centreon.com/centreon-collect-centos7-dependencies:21.10'
        sh 'rpmsign --addsign *.rpm'
        stash name: 'el7-rpms', includes: '*.rpm'
        archiveArtifacts artifacts: "*.rpm"
        sh 'rm -rf *.rpm'
      } 
    }
  },
  'centos8 Build and UT': {
    node("C++") {
      dir('centreon-engine-centos8') {
        checkout scm
        sh 'docker run -i --entrypoint /src/ci/scripts/engine-unit-tests.sh -v "$PWD:/src" registry.centreon.com/centreon-collect-centos8-dependencies:21.10'
      }
    }
  },
  'centos8 rpm packaging and signing': {
    node("C++") {
      dir('centreon-engine-centos8') {
        checkout scm
        sh 'docker run -i --entrypoint /src/centreon-engine/ci/scripts/engine-rpm-package.sh -v "$PWD:/src/centreon-engine" -e DISTRIB="el8" -e VERSION=$VERSION -e RELEASE=$RELEASE registry.centreon.com/centreon-collect-centos8-dependencies:21.10'
        sh 'rpmsign --addsign *.rpm'
        stash name: 'el8-rpms', includes: '*.rpm'
        archiveArtifacts artifacts: "*.rpm"
        sh 'rm -rf *.rpm'
      }
    }
  },
  'debian buster Build and UT': {
    node("C++") {
      dir('centreon-engine-debian') {
        checkout scm
        sh 'docker run -i --entrypoint /src/ci/scripts/engine-unit-tests.sh -v "$PWD:/src" registry.centreon.com/centreon-collect-debian-dependencies:21.10'
      }
    }
  },
  'debian buster packaging and signing': {
    node("C++") {
      dir('centreon-engine-centos8') {
        //checkout scm
        //sh 'docker run -i --entrypoint /src/ci/scripts/engine-rpm-package.sh -v "$PWD:/src" -e DISTRIB="el8" -e VERSION=$VERSION -e RELEASE=$RELEASE registry.centreon.com/centreon-collect-centos8-dependencies:21.10'
        //sh 'rpmsign --addsign *.rpm'
        //stash name: 'el8-rpms', includes: '*.rpm'
        //archiveArtifacts artifacts: "*.rpm"
        //sh 'rm -rf *.rpm'
      }
    }
  } 
}

if ((env.BUILD == 'RELEASE') || (env.BUILD == 'QA')) {
  stage('Delivery') {
    node("C++") {
      unstash 'el8-rpms'
      unstash 'el7-rpms'
      dir('centreon-engine-delivery') {
        checkout scm
        loadCommonScripts()
        sh 'rm -rf output && mkdir output && mv ../*.rpm output'
        sh './ci/scripts/engine-rpm-delivery.sh'
      }
    }
  }
}