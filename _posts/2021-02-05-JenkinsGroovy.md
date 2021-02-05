---
layout: post
title:  "Simple note for writing a Jenkinsfile"
date:   2021-02-05
comments: true
excerpt: "Jenkins Groovy"
categories: CI/CD
tag:
- CI/CD 
---

# Required fields of Jenkinsfile
```groovy
pipeline { // must included
  agent any // run next available agnet

  stages { // where the "work" happens
    stage("build") {
      steps {
        sh 'npm install'
        sh 'npm build'
      }
    }

    stage("test") {
      steps {

      }
    }

    stage("deploy") {
      steps {

      }
    }
  }
}

node {

}
```
# Additonal fileds 
```groovy
CODE_CHANGES = getGitChanges()
pipeline { // must included
  ...

  parameters { // Parameterize your Build
    string(name: 'VERSION', defaultValue: '', description: 'version to deploy on prod')
    choice(name: 'VERSION', choices: ['1.1.0', '1.2.0', '1.3.0'], description: ''])
    booleanParam(name: 'executeTests', defaultValue: true, description: '')
  }

  tools { // Access build tools for your projects
    maven 'Maven' // defined in Global Tool Configuration
    gradle 'Gradle'
    jdk 'Jdk'
  }

  stages {
    ...
    stage("test") {
      when {
        expression {
          BRANCH_NAME == 'dev' || BRANCH_NAME == 'master' && CODE_CHANGES == true
          params.executeTests
        }
      }
      steps {
        ...
        echo '${params.VERSION}'
      }
      ...
    }
    ...
  }

  post { //Execute some logic after all stages executed
    always {

    }
    failure {

    }
    success {

    }
  }
}
```
# Environmental variables
`${JENKINS_URL}`/env-vars.html/ 에서 사용가능한 환경변수를 확인할 수 있다.
```groovy
...
pipeline { // must included
  ...
  environment { // use custom env variables
    NEW_VERSION = '1.3.0'
    SERVER_CREDENTIALS = credentials('')
  }
  stages {
    ...
    
    stage("build") {
      steps {
        echo "building vesrion ${NEW_VERSION}" // javascript의 ` 대신 " 사용  
        withCredentials([
          usernamePassword(
          credentials : 'server-credentials',
          usernameVariable : USER,
          passwordVariable : PWD)
        ]) {
          sh "some script ${USER} ${PWD}"
        }
      }
    }
    ...
  }
  ...
}
```

# External Scripts 
## Jenkins
```groovy
...
pipeline { // must included
  ...
  stages {
    ...
    stage("init") {
      steps {
        script {
          gv =load "script.groovy"
        }
      }
    }
    
    stage("build") {
      steps {
        script {
          gv.buildApp()
        }
      }
    }
    ...
  }
  ...
}
```

## script.groovy
```groovy
def buildApp() {
  echo 'building the application...'
  // All environmental variables in JenkinsFile are available in the groovy script
  echo "building version ${params.VERSION}"
}
return this
```
