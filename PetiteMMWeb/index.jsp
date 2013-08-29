<%@ page contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<!DOCTYPE html>
<html lang="ja">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <link rel="stylesheet" type="text/css" href="test.css">
  <title>test</title>
</head>
<body>
  <h1>PetiteMM</h1>
  <form method="POST" enctype="multipart/form-data" action="./PetiteMM">
    <ul>
      <li><input type="file" name="midi" size="75" /></li>
      <li><input type="submit" value="Upload" /></li>
    </ul>
  </form>
</body>
</html>
