<%@ page trimDirectiveWhitespaces="true"%>
<%@ page contentType="text/html; charset=UTF-8" pageEncoding="UTF-8"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<jsp:useBean id="petiteMMBean" scope="request" class="com.googlecode.loveemu.PetiteMM.web.PetiteMMBean" />
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
      <li>MML timebase (TPQN) <select name="timebase">
        <option value="12">12</option>
        <option value="24" selected="selected">24</option>
        <option value="48">48</option>
        <option value="96">96</option>
        <option value="192">192</option>
        <option value="384">384</option>
        <option value="0">Same as input</option>
      </select></li>
      <li>Max number of dots <select name="dots">
        <option value="0">12</option>
        <option value="1">24</option>
        <option value="2" selected="selected">2</option>
        <option value="3">3</option>
        <option value="-1">No limit</option>
      </select></li>
      <li><input type="checkbox" name="octaveReverse" value="1" /> Reverse octave</li>
      <li><input type="checkbox" name="useTriplet" value="1" /> Use triplet</li>
    </ul>
  </form>
  <p><textarea name="mml" rows="24" cols="80"><c:out value="${petiteMMBean.mml}" default="" /></textarea></p>
  <c:if test="${not empty petiteMMBean.errorMessage}">
    <pre style="color: red;"><c:out value="${petiteMMBean.errorMessage}" default="" /></pre>
  </c:if>
</body>
</html>
