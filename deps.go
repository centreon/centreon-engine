package main

import (
    "bufio"
    "fmt"
    "log"
    "os"
    "path/filepath"
    "regexp"
    "strings"
    )

// MaxDepth Max depth in search tree
const MaxDepth = 3

func findIncludes(file string, treated *[]string, edges *[]string, depth int) {
  var myList []string
  file1 := file
  f, err := os.Open(file1)
  if err != nil {
    file1 = strings.TrimPrefix(file, "inc/")
    for _, pref := range []string{
                            "/usr/local/include/",
                            "inc/",
                            "modules/external_commands/inc/" } {
      f, err = os.Open(pref + file1)
      if err == nil {
        file1 = pref + file1
        *treated = append(*treated, file1)
        break
      }
    }
  } else {
    *treated = append(*treated, file1)
  }
  defer f.Close()

  depth++
  if depth > MaxDepth {
    return
  }

  scanner := bufio.NewScanner(f)
  r, _ := regexp.Compile("^#\\s*include\\s*([<\"])(.*)[>\"]")
  for scanner.Scan() {
    line := scanner.Text()
    match := r.FindStringSubmatch(line)
    if len(match) > 0 {
    /* match[0] is the global match, match[1] is '<' or '"' and match[2] is the file to include */
      if match[1] == "\"" {
        *edges = append(*edges, fmt.Sprintf("  \"%s\" -> \"%s\";\n", file, match[2]))
        myList = append(myList, match[2])
      } else {
        *edges = append(*edges, fmt.Sprintf("  \"%s\" -> \"%s\";\n", file, match[2]))
      }
    }
  }
  if err := scanner.Err(); err != nil {
    log.Print(file, " --- ", err)
  }

  for _, file2 := range myList {
    found := false
    for _, ff := range *treated {
      if ff == file2 {
        found = true
        break
      }
    }
    if !found {
      findIncludes(file2, treated, edges, depth)
    }
  }
}

func unique(edges []string) []string {
    keys := make(map[string]bool)
    list := []string{}
    for _, entry := range edges {
        if _, value := keys[entry]; !value {
            keys[entry] = true
            list = append(list, entry)
        }
    }
    return list
}

func main() {
  args := os.Args[1:]
  var fileList []string
  var edges []string

  if len(args) == 0 {
    for _, searchDir := range []string{"src", "inc"} {
      filepath.Walk(searchDir, func(path string, f os.FileInfo, err error) error {
        if strings.HasSuffix(path, ".cc") || strings.HasSuffix(path, ".hh") {
          fileList = append(fileList, path)
        }
        return nil
      })
    }
  } else {
    fileList = append(fileList, args[0])
  }

  fmt.Println("digraph deps {")

  var treated []string
  for _, file := range fileList {
    findIncludes(file, &treated, &edges, 0)
  }
  edges = unique(edges)
  for _, l := range edges {
    fmt.Println(l)
  }
  fmt.Println("}")
}
