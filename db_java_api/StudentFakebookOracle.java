package project2;

import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.ResultSet;
import java.util.ArrayList;

/*
    The StudentFakebookOracle class is derived from the FakebookOracle class and implements
    the abstract query functions that investigate the database provided via the <connection>
    parameter of the constructor to discover specific information.
*/
public final class StudentFakebookOracle extends FakebookOracle {
    // [Constructor]
    // REQUIRES: <connection> is a valid JDBC connection
    public StudentFakebookOracle(Connection connection) {
        oracle = connection;
    }

    @Override
    // Query 0
    // -----------------------------------------------------------------------------------
    // GOALS: (A) Find the total number of users for which a birth month is listed
    //        (B) Find the birth month in which the most users were born
    //        (C) Find the birth month in which the fewest users (at least one) were born
    //        (D) Find the IDs, first names, and last names of users born in the month
    //            identified in (B)
    //        (E) Find the IDs, first names, and last name of users born in the month
    //            identified in (C)
    //
    // This query is provided to you completed for reference. Below you will find the appropriate
    // mechanisms for opening up a statement, executing a query, walking through results, extracting
    // data, and more things that you will need to do for the remaining nine queries
    public BirthMonthInfo findMonthOfBirthInfo() throws SQLException {
        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            // Step 1
            // ------------
            // * Find the total number of users with birth month info
            // * Find the month in which the most users were born
            // * Find the month in which the fewest (but at least 1) users were born
            ResultSet rst = stmt.executeQuery(
                    "SELECT COUNT(*) AS Birthed, Month_of_Birth " + // select birth months and number of uses with that birth month
                            "FROM " + UsersTable + " " + // from all users
                            "WHERE Month_of_Birth IS NOT NULL " + // for which a birth month is available
                            "GROUP BY Month_of_Birth " + // group into buckets by birth month
                            "ORDER BY Birthed DESC, Month_of_Birth ASC"); // sort by users born in that month, descending; break ties by birth month

            int mostMonth = 0;
            int leastMonth = 0;
            int total = 0;
            while (rst.next()) { // step through result rows/records one by one
                if (rst.isFirst()) { // if first record
                    mostMonth = rst.getInt(2); //   it is the month with the most
                }
                if (rst.isLast()) { // if last record
                    leastMonth = rst.getInt(2); //   it is the month with the least
                }
                total += rst.getInt(1); // get the first field's value as an integer
            }
            BirthMonthInfo info = new BirthMonthInfo(total, mostMonth, leastMonth);

            // Step 2
            // ------------
            // * Get the names of users born in the most popular birth month
            rst = stmt.executeQuery(
                    "SELECT User_ID, First_Name, Last_Name " + // select ID, first name, and last name
                            "FROM " + UsersTable + " " + // from all users
                            "WHERE Month_of_Birth = " + mostMonth + " " + // born in the most popular birth month
                            "ORDER BY User_ID"); // sort smaller IDs first

            while (rst.next()) {
                info.addMostPopularBirthMonthUser(new UserInfo(rst.getLong(1), rst.getString(2), rst.getString(3)));
            }

            // Step 3
            // ------------
            // * Get the names of users born in the least popular birth month
            rst = stmt.executeQuery(
                    "SELECT User_ID, First_Name, Last_Name " + // select ID, first name, and last name
                            "FROM " + UsersTable + " " + // from all users
                            "WHERE Month_of_Birth = " + leastMonth + " " + // born in the least popular birth month
                            "ORDER BY User_ID"); // sort smaller IDs first

            while (rst.next()) {
                info.addLeastPopularBirthMonthUser(new UserInfo(rst.getLong(1), rst.getString(2), rst.getString(3)));
            }

            // Step 4
            // ------------
            // * Close resources being used
            rst.close();
            stmt.close(); // if you close the statement first, the result set gets closed automatically

            return info;

        } catch (SQLException e) {
            System.err.println(e.getMessage());
            return new BirthMonthInfo(-1, -1, -1);
        }
    }

    @Override
    // Query 1
    // -----------------------------------------------------------------------------------
    // GOALS: (A) The first name(s) with the most letters
    //        (B) The first name(s) with the fewest letters
    //        (C) The first name held by the most users
    //        (D) The number of users whose first name is that identified in (C)
    public FirstNameInfo findNameInfo() throws SQLException {
        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            /*
                EXAMPLE DATA STRUCTURE USAGE
                ============================================
                FirstNameInfo info = new FirstNameInfo();
                info.addLongName("Aristophanes");
                info.addLongName("Michelangelo");
                info.addLongName("Peisistratos");
                info.addShortName("Bob");
                info.addShortName("Sue");
                info.addCommonName("Harold");
                info.addCommonName("Jessica");
                info.setCommonNameCount(42);
                return info;
            */
            int lenLongest = 0;
            int lenShortest = 0;
            int commonCnt = 0;
            FirstNameInfo info = new FirstNameInfo();

            // Step 1
            // ------------
            // * Get the longest first names by length
            ResultSet rst = stmt.executeQuery(
                "SELECT DISTINCT FIRST_NAME, LENGTH(FIRST_NAME) AS lenfirstn " + 
                "FROM " + UsersTable + " " + 
                "ORDER BY LENGTH(FIRST_NAME) DESC, FIRST_NAME ASC"
                );

            while (rst.next()) {
                if (rst.isFirst()) {
                    lenLongest = rst.getInt(2);
                }
                if(rst.getInt(2) == lenLongest) {
                    info.addLongName(rst.getString(1));
                }
            }

            // Step 2
            // ------------
            // * Get the shortest first names by length
            rst = stmt.executeQuery(
                "SELECT DISTINCT FIRST_NAME, LENGTH(FIRST_NAME) AS lenfirstn " + 
                "FROM " + UsersTable + " " + 
                "ORDER BY LENGTH(FIRST_NAME) ASC, FIRST_NAME ASC"
                );
            
            while (rst.next()) {
                if (rst.isFirst()) {
                    lenShortest = rst.getInt(2);
                }
                if(rst.getInt(2) == lenShortest) {
                    info.addShortName(rst.getString(1));
                }
            }

            // Step 3
            // ------------
            // * Get the most common first names and the count
            rst = stmt.executeQuery(
                "SELECT FIRST_NAME, COUNT(*) AS nuser " + 
                "FROM " + UsersTable + " " + 
                "GROUP BY FIRST_NAME " + 
                "ORDER BY COUNT(*) DESC, FIRST_NAME ASC"
                );
            while (rst.next()) {
                if (rst.isFirst()) {
                    info.setCommonNameCount(rst.getInt(2));
                    commonCnt = rst.getInt(2);
                }
                if(rst.getInt(2) == commonCnt) {
                    info.addCommonName(rst.getString(1));
                }
            }

            // Step 4
            // ------------
            // * Close resources being used
            rst.close();
            stmt.close();

            return info;
            //return new FirstNameInfo(); // placeholder for compilation
        } catch (SQLException e) {
            System.err.println(e.getMessage());
            return new FirstNameInfo();
        }
    }

    @Override
    // Query 2
    // -----------------------------------------------------------------------------------
    // GOALS: (A) Find the IDs, first names, and last names of users without any friends
    //
    // Be careful! Remember that if two users are friends, the Friends table only contains
    // the one entry (U1, U2) where U1 < U2.
    public FakebookArrayList<UserInfo> lonelyUsers() throws SQLException {
        FakebookArrayList<UserInfo> results = new FakebookArrayList<UserInfo>(", ");

        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            /*
                EXAMPLE DATA STRUCTURE USAGE
                ============================================
                UserInfo u1 = new UserInfo(15, "Abraham", "Lincoln");
                UserInfo u2 = new UserInfo(39, "Margaret", "Thatcher");
                results.add(u1);
                results.add(u2);
            */

            ResultSet rst = stmt.executeQuery(
                "SELECT U.USER_ID, U.FIRST_NAME, U.LAST_NAME " + 
                "FROM " + UsersTable + " U " +
                "LEFT OUTER JOIN " + FriendsTable + " F1 ON U.USER_ID = F1.USER1_ID " +
                "LEFT OUTER JOIN " + FriendsTable + " F2 ON U.USER_ID = F2.USER2_ID " +
                "WHERE F1.USER1_ID IS NULL AND F2.USER2_ID IS NULL"
                );

            while (rst.next()) {
                UserInfo user = new UserInfo(rst.getLong(1), rst.getString(2), rst.getString(3));
                results.add(user);
            }

            // * Close resources being used
            rst.close();
            stmt.close(); // if you close the statement first, the result set gets closed automatically

        } catch (SQLException e) {
            System.err.println(e.getMessage());
        }

        return results;
    }

    @Override
    // Query 3
    // -----------------------------------------------------------------------------------
    // GOALS: (A) Find the IDs, first names, and last names of users who no longer live
    //            in their hometown (i.e. their current city and their hometown are different)
    public FakebookArrayList<UserInfo> liveAwayFromHome() throws SQLException {
        FakebookArrayList<UserInfo> results = new FakebookArrayList<UserInfo>(", ");

        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            /*
                EXAMPLE DATA STRUCTURE USAGE
                ============================================
                UserInfo u1 = new UserInfo(9, "Meryl", "Streep");
                UserInfo u2 = new UserInfo(104, "Tom", "Hanks");
                results.add(u1);
                results.add(u2);
            */

            ResultSet rst = stmt.executeQuery(
                "SELECT DISTINCT U.USER_ID, U.FIRST_NAME, U.LAST_NAME " + 
                "FROM " + UsersTable + " U " +
                "LEFT OUTER JOIN " + CurrentCitiesTable + " C " + 
                    "ON (U.USER_ID = C.USER_ID) " + 
                "LEFT OUTER JOIN " + HometownCitiesTable + " H " + 
                    "ON (U.USER_ID = H.USER_ID) " + 
                "WHERE C.CURRENT_CITY_ID IS NOT NULL AND " + 
                    "H.HOMETOWN_CITY_ID IS NOT NULL AND " + 
                    "C.CURRENT_CITY_ID != H.HOMETOWN_CITY_ID " + 
                "ORDER BY U.USER_ID ASC" 
                );

            while (rst.next()) {
                UserInfo user = new UserInfo(rst.getLong(1), rst.getString(2), rst.getString(3));
                results.add(user);
            }

            // * Close resources being used
            rst.close();
            stmt.close(); // if you close the statement first, the result set gets closed automatically

        } catch (SQLException e) {
            System.err.println(e.getMessage());
        }

        return results;
    }

    @Override
    // Query 4
    // -----------------------------------------------------------------------------------
    // GOALS: (A) Find the IDs, links, and IDs and names of the containing album of the top
    //            <num> photos with the most tagged users
    //        (B) For each photo identified in (A), find the IDs, first names, and last names
    //            of the users therein tagged
    public FakebookArrayList<TaggedPhotoInfo> findPhotosWithMostTags(int num) throws SQLException {
        FakebookArrayList<TaggedPhotoInfo> results = new FakebookArrayList<TaggedPhotoInfo>("\n");

        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            /*
                EXAMPLE DATA STRUCTURE USAGE
                ============================================
                PhotoInfo p = new PhotoInfo(80, 5, "www.photolink.net", "Winterfell S1");
                UserInfo u1 = new UserInfo(3901, "Jon", "Snow");
                UserInfo u2 = new UserInfo(3902, "Arya", "Stark");
                UserInfo u3 = new UserInfo(3903, "Sansa", "Stark");
                TaggedPhotoInfo tp = new TaggedPhotoInfo(p);
                tp.addTaggedUser(u1);
                tp.addTaggedUser(u2);
                tp.addTaggedUser(u3);
                results.add(tp);
            */

        //  Step 1
        // ------------
        // * (A) Find the IDs, links, and IDs and names of the containing 
        // album of the top <num> photos with the most tagged users
        ResultSet rst = stmt.executeQuery(
            "SELECT DISTINCT TP.ntag, P.PHOTO_ID, P.PHOTO_LINK, A.ALBUM_ID, A.ALBUM_NAME " + 
            "FROM (SELECT * FROM  " + 
                    "(SELECT TAG_PHOTO_ID, COUNT(DISTINCT TAG_SUBJECT_ID) AS ntag " + 
                    "FROM " + TagsTable + " " + 
                    "GROUP BY TAG_PHOTO_ID " + 
                    "ORDER BY COUNT(DISTINCT TAG_SUBJECT_ID) DESC, TAG_PHOTO_ID ASC) " + 
                  "WHERE ROWNUM <= " + num + ") TP " + 
            "INNER JOIN " + PhotosTable + " P " + 
                "ON(TP.TAG_PHOTO_ID = P.PHOTO_ID) " +
            "INNER JOIN " + AlbumsTable + " A " + 
                "ON(P.ALBUM_ID = A.ALBUM_ID) " +
            "ORDER BY TP.ntag DESC, P.PHOTO_ID ASC"
            );

        while (rst.next()) {
            PhotoInfo p = new PhotoInfo(rst.getInt(2), rst.getInt(4), rst.getString(3), rst.getString(5));
            TaggedPhotoInfo result = new TaggedPhotoInfo(p);

            //  Step 2
            // ------------
            // * (B) For each photo identified in (A), find the IDs, first 
            // names, and last names of the users therein tagged

            Statement stmt2 = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly);
            ResultSet rst2 = stmt2.executeQuery(
                "SELECT DISTINCT " + 
                    "U.USER_ID, U.FIRST_NAME, U.LAST_NAME " + 
                "FROM " + TagsTable + " T " + 
                "INNER JOIN " + UsersTable + " U " + 
                    "ON (T.TAG_SUBJECT_ID = U.USER_ID) " + 
                "WHERE T.TAG_PHOTO_ID = " + rst.getInt(2) + " " + 
                "ORDER BY U.USER_ID ASC"
                );

            while(rst2.next()) {
                UserInfo user = new UserInfo(rst2.getInt(1), rst2.getString(2), rst2.getString(3));
                result.addTaggedUser(user);
            }

            rst2.close();
            stmt2.close(); 
            results.add(result);
        }

        // Step 3
        // ------------
        // * Close resources being used
        rst.close();
        stmt.close(); 


        } catch (SQLException e) {
            System.err.println(e.getMessage());
        }

        return results;
    }

    @Override
    // Query 5
    // -----------------------------------------------------------------------------------
    // GOALS: (A) Find the IDs, first names, last names, and birth years of each of the two
    //            users in the top <num> pairs of users that meet each of the following
    //            criteria:
    //              (i) same gender
    //              (ii) tagged in at least one common photo
    //              (iii) difference in birth years is no more than <yearDiff>
    //              (iv) not friends
    //        (B) For each pair identified in (A), find the IDs, links, and IDs and names of
    //            the containing album of each photo in which they are tagged together
    public FakebookArrayList<MatchPair> matchMaker(int num, int yearDiff) throws SQLException {
        FakebookArrayList<MatchPair> results = new FakebookArrayList<MatchPair>("\n");

        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            /*
                EXAMPLE DATA STRUCTURE USAGE
                ============================================
                UserInfo u1 = new UserInfo(93103, "Romeo", "Montague");
                UserInfo u2 = new UserInfo(93113, "Juliet", "Capulet");
                MatchPair mp = new MatchPair(u1, 1597, u2, 1597);
                PhotoInfo p = new PhotoInfo(167, 309, "www.photolink.net", "Tragedy");
                mp.addSharedPhoto(p);
                results.add(mp);
            */

            //  Step 1
            // ------------
            // (A) Find the IDs, first names, last names, and birth years of 
            // each of the two users in the top <num> pairs of users that 
            // meet each of the following criteria:
            // (i) same gender
            // (ii) tagged in at least one common photo
            // (iii) difference in birth years is no more than <yearDiff>
            // (iv) not friends

            ResultSet rst = stmt.executeQuery(
                "SELECT * FROM (  " + 
                    "SELECT DISTINCT U1.USER_ID AS UID1,   " + 
                            "U1.FIRST_NAME AS FIRSTN1,   " + 
                            "U1.LAST_NAME AS LASTN1,   " + 
                            "U1.YEAR_OF_BIRTH AS YEAR1,  " + 
                            "U2.USER_ID AS UID2,   " + 
                            "U2.FIRST_NAME AS FIRSTN2,   " + 
                            "U2.LAST_NAME AS LASTN2,   " + 
                            "U2.YEAR_OF_BIRTH AS YEAR2,  " + 
                            "COUNT(DISTINCT T1.TAG_PHOTO_ID) AS ntag  " + 
                    "FROM " + UsersTable + " U1,  " + 
                        UsersTable + " U2,  " + 
                        TagsTable + " T1,  " + 
                        TagsTable + " T2,  " + 
                        FriendsTable + " F " + 
                    "WHERE U1.GENDER = U2.GENDER AND   " + 
                        "U1.USER_ID = T1.TAG_SUBJECT_ID AND  " + 
                        "U2.USER_ID = T2.TAG_SUBJECT_ID AND  " + 
                        "T1.TAG_PHOTO_ID = T2.TAG_PHOTO_ID AND  " + 
                        "U1.USER_ID < U2.USER_ID AND  " + 
                        "((U1.USER_ID = F.USER1_ID AND U2.USER_ID != F.USER2_ID) OR   " + 
                        "(U1.USER_ID != F.USER1_ID AND U2.USER_ID = F.USER2_ID)) AND   " + 
                        "ABS(U1.YEAR_OF_BIRTH - U2.YEAR_OF_BIRTH) <= " + yearDiff + " " + 
                    "GROUP BY U1.USER_ID, U1.FIRST_NAME, U1.LAST_NAME, U1.YEAR_OF_BIRTH, " + 
                        "U2.USER_ID, U2.FIRST_NAME, U2.LAST_NAME , U2.YEAR_OF_BIRTH " + 
                    "ORDER BY COUNT(DISTINCT T1.TAG_PHOTO_ID) DESC,  " + 
                        "U1.USER_ID ASC, U2.USER_ID ASC)  " + 
                "WHERE ROWNUM <= " + num + " "
                );

            while (rst.next()) {
                UserInfo u1 = new UserInfo(rst.getInt(1), rst.getString(2), rst.getString(3));
                UserInfo u2 = new UserInfo(rst.getInt(5), rst.getString(6), rst.getString(7));
                MatchPair mp = new MatchPair(u1, rst.getInt(4), u2, rst.getInt(8));
                
                //  Step 2
                // ------------
                // For each pair identified in (A), find the IDs, links, and IDs
                // and names of the containing album of each photo in which 
                // they are tagged together
                
                Statement stmt2 = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly);
                ResultSet rst2 = stmt2.executeQuery(
                    "SELECT P.PHOTO_ID, P.PHOTO_LINK, A.ALBUM_ID, A.ALBUM_NAME  "  + 
                        "FROM " + TagsTable + " T1 " + 
                        "INNER JOIN " + TagsTable + " T2 " + 
                            "ON (T1.TAG_PHOTO_ID = T2.TAG_PHOTO_ID)  "  + 
                        "INNER JOIN " + PhotosTable + " P  " + 
                            "ON (T1.TAG_PHOTO_ID = P.PHOTO_ID)  "  + 
                        "INNER JOIN " + AlbumsTable + " A  " + 
                            "ON (P.ALBUM_ID = A.ALBUM_ID)  "  + 
                        "WHERE T1.TAG_SUBJECT_ID = " + rst.getInt(1) + " AND " + 
                            "T2.TAG_SUBJECT_ID = " + rst.getInt(5) + " " + 
                        "ORDER BY P.PHOTO_ID  "
                    );

                while(rst2.next()) {
                    PhotoInfo p = new PhotoInfo(rst2.getInt(1), rst2.getInt(3), rst2.getString(2), rst2.getString(4));
                    mp.addSharedPhoto(p);
                }

                rst2.close();
                stmt2.close(); 
                results.add(mp);
            }

            // Step 3
            // ------------
            // * Close resources being used
            rst.close();
            stmt.close(); 

        } catch (SQLException e) {
            System.err.println(e.getMessage());
        }

        return results;
    }

    @Override
    // Query 6
    // -----------------------------------------------------------------------------------
    // GOALS: (A) Find the IDs, first names, and last names of each of the two users in
    //            the top <num> pairs of users who are not friends but have a lot of
    //            common friends
    //        (B) For each pair identified in (A), find the IDs, first names, and last names
    //            of all the two users' common friends
    public FakebookArrayList<UsersPair> suggestFriends(int num) throws SQLException {
        FakebookArrayList<UsersPair> results = new FakebookArrayList<UsersPair>("\n");

        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            /*
                EXAMPLE DATA STRUCTURE USAGE
                ============================================
                UserInfo u1 = new UserInfo(16, "The", "Hacker");
                UserInfo u2 = new UserInfo(80, "Dr.", "Marbles");
                UserInfo u3 = new UserInfo(192, "Digit", "Le Boid");
                UsersPair up = new UsersPair(u1, u2);
                up.addSharedFriend(u3);
                results.add(up);
            */

            //  Step 1
            // ------------
            // A) Find the IDs, first names, and last names of each of the two users in
            // the top <num> pairs of users who are not friends but have a lot of
            // common friends

            ResultSet rst = stmt.executeQuery(
                "SELECT UID1, FIRSTN1, LASTN1, UID2, FIRSTN2, LASTN2 " + 
                "FROM ( " + 
                    "SELECT U1.USER_ID AS UID1, U1.FIRST_NAME AS FIRSTN1, U1.LAST_NAME AS LASTN1, " + 
                        "U2.USER_ID AS UID2, U2.FIRST_NAME AS FIRSTN2, U2.LAST_NAME AS LASTN2, " + 
                        "COUNT(DISTINCT CONCAT(CONCAT(U1.USER_ID, U2.USER_ID), CASE WHEN (U1.USER_ID = F1.USER1_ID AND U2.USER_ID = F2.USER2_ID) " + 
                                        "OR (U1.USER_ID = F1.USER1_ID AND U2.USER_ID = F2.USER1_ID) " + 
                                    "THEN F1.USER2_ID " + 
                                    "WHEN (U1.USER_ID = F1.USER2_ID AND U2.USER_ID = F2.USER2_ID) " + 
                                        "OR (U1.USER_ID = F1.USER2_ID AND U2.USER_ID = F2.USER1_ID) " + 
                                    "THEN F1.USER1_ID " + 
                                    "END)) AS nmutual " + 
                    "FROM (SELECT USER_ID, FIRST_NAME, LAST_NAME FROM " + UsersTable + ") U1, " + 
                        "(SELECT USER_ID,  FIRST_NAME, LAST_NAME FROM " + UsersTable + ") U2, " + 
                        FriendsTable + " F1, " + 
                        FriendsTable + " F2 " + 
                    "WHERE (U1.USER_ID = F1.USER1_ID AND " + 
                        "U2.USER_ID = F2.USER2_ID AND " + 
                        "F1.USER2_ID = F2.USER1_ID AND " + 
                        "U1.USER_ID < U2.USER_ID) OR " + 
                        "(U1.USER_ID = F1.USER1_ID AND " + 
                        "U2.USER_ID = F2.USER1_ID AND " + 
                        "F1.USER2_ID = F2.USER2_ID AND " + 
                        "U1.USER_ID < U2.USER_ID) OR " + 
                        "(U1.USER_ID = F1.USER2_ID AND " + 
                        "U2.USER_ID = F2.USER2_ID AND " + 
                        "F1.USER1_ID = F2.USER1_ID AND " + 
                        "U1.USER_ID < U2.USER_ID) OR " + 
                        "(U1.USER_ID = F1.USER2_ID AND " + 
                        "U2.USER_ID = F2.USER1_ID AND " + 
                        "F1.USER1_ID = F2.USER2_ID AND " + 
                        "U1.USER_ID < U2.USER_ID) " +
                    "GROUP BY U1.USER_ID, U1.FIRST_NAME, U1.LAST_NAME, " + 
                        "U2.USER_ID, U2.FIRST_NAME, U2.LAST_NAME " + 
                    "ORDER BY COUNT(DISTINCT CONCAT(CONCAT(U1.USER_ID, U2.USER_ID), CASE WHEN (U1.USER_ID = F1.USER1_ID AND U2.USER_ID = F2.USER2_ID) " + 
                                        "OR (U1.USER_ID = F1.USER1_ID AND U2.USER_ID = F2.USER1_ID) " + 
                                    "THEN F1.USER2_ID " + 
                                    "WHEN (U1.USER_ID = F1.USER2_ID AND U2.USER_ID = F2.USER2_ID) " + 
                                        "OR (U1.USER_ID = F1.USER2_ID AND U2.USER_ID = F2.USER1_ID) " + 
                                    "THEN F1.USER1_ID " + 
                                    "END)) DESC,  " + 
                        "U1.USER_ID ASC, U2.USER_ID ASC) " + 
                "WHERE ROWNUM <= " + num + " AND CONCAT(UID1, UID2) NOT IN " + 
                "(SELECT CONCAT(U1.USER_ID, U2.USER_ID) " + 
                "FROM project2.PUBLIC_USERS U1,  " + 
                    "project2.PUBLIC_USERS U2, " + 
                    "Project2.PUBLIC_FRIENDS F " + 
                "WHERE U1.USER_ID = F.USER1_ID AND U2.USER_ID = F.USER2_ID " + 
                    "OR U1.USER_ID = F.USER2_ID AND U2.USER_ID = F.USER1_ID  " + 
                    "AND U1.USER_ID < U2.USER_ID) "

            );

            while (rst.next()) {
                UserInfo u1 = new UserInfo(rst.getInt(1), rst.getString(2), rst.getString(3));
                UserInfo u2 = new UserInfo(rst.getInt(4), rst.getString(5), rst.getString(6));
                UsersPair up = new UsersPair(u1, u2);

                //  Step 2
                // ------------
                // For each pair identified in (A), find the IDs, first names, and last names
                // of all the two users' common friends

                Statement stmt2 = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly);
                ResultSet rst2 = stmt2.executeQuery(
                    "SELECT UM.UIDM, UU.FIRST_NAME, UU.LAST_NAME " + 
                    "FROM ( " + 
                        "SELECT DISTINCT " + 
                                "CASE WHEN (U1.USER_ID = F1.USER1_ID AND U2.USER_ID = F2.USER2_ID) " + 
                                        "OR (U1.USER_ID = F1.USER1_ID AND U2.USER_ID = F2.USER1_ID) " + 
                                    "THEN F1.USER2_ID " + 
                                    "WHEN (U1.USER_ID = F1.USER2_ID AND U2.USER_ID = F2.USER2_ID) " + 
                                        "OR (U1.USER_ID = F1.USER2_ID AND U2.USER_ID = F2.USER1_ID) " + 
                                    "THEN F1.USER1_ID " + 
                                    "END AS UIDM " + 
                        "FROM (SELECT USER_ID, FIRST_NAME, LAST_NAME FROM " + UsersTable + " " + 
                             "WHERE USER_ID = " + rst.getInt(1) + ") U1, " + 
                            "(SELECT USER_ID, FIRST_NAME, LAST_NAME FROM " + UsersTable + " " + 
                             "WHERE USER_ID = " + rst.getInt(4) + ") U2, " + 
                            "(SELECT USER1_ID, USER2_ID FROM " + FriendsTable + " " + 
                                "WHERE USER1_ID = " + rst.getInt(1) + " " + 
                                "OR USER1_ID = " + rst.getInt(4) + " " + 
                                "OR USER2_ID = " + rst.getInt(1) + " " + 
                                "OR USER2_ID = " + rst.getInt(4) + " ) F1, " + 
                            "(SELECT USER1_ID, USER2_ID FROM " + FriendsTable + " " + 
                                "WHERE USER1_ID = " + rst.getInt(1) + " " + 
                                "OR USER1_ID = " + rst.getInt(4) + " " + 
                                "OR USER2_ID = " + rst.getInt(1) + " " + 
                                "OR USER2_ID = " + rst.getInt(4) + " ) F2 " + 
                        "WHERE (U1.USER_ID = F1.USER1_ID AND " + 
                            "U2.USER_ID = F2.USER2_ID AND " + 
                            "F1.USER2_ID = F2.USER1_ID AND " + 
                            "U1.USER_ID < U2.USER_ID) OR " + 
                            "(U1.USER_ID = F1.USER1_ID AND  " + 
                            "U2.USER_ID = F2.USER1_ID AND " + 
                            "F1.USER2_ID = F2.USER2_ID AND " + 
                            "U1.USER_ID < U2.USER_ID) OR  " + 
                            "(U1.USER_ID = F1.USER2_ID AND " + 
                            "U2.USER_ID = F2.USER2_ID AND " + 
                            "F1.USER1_ID = F2.USER1_ID AND " + 
                            "U1.USER_ID < U2.USER_ID) OR " + 
                            "(U1.USER_ID = F1.USER2_ID AND " + 
                            "U2.USER_ID = F2.USER1_ID AND " + 
                            "F1.USER1_ID = F2.USER2_ID AND " + 
                            "U1.USER_ID < U2.USER_ID)) UM " + 
                    "INNER JOIN " + UsersTable + " UU ON (UM.UIDM = UU.USER_ID) " + 
                    "ORDER BY UM.UIDM ASC "
                );

                while(rst2.next()) {
                    UserInfo u3 = new UserInfo(rst2.getInt(1), rst2.getString(2), rst2.getString(3));
                    up.addSharedFriend(u3);
                }
                results.add(up); 
                rst2.close();
                stmt2.close(); 
            }

            // Step 3
            // ------------
            // * Close resources being used
            rst.close();
            stmt.close(); 

        } catch (SQLException e) {
            System.err.println(e.getMessage());
        }

        return results;
    }

    @Override
    // Query 7
    // -----------------------------------------------------------------------------------
    // GOALS: (A) Find the name of the state or states in which the most events are held
    //        (B) Find the number of events held in the states identified in (A)
    public EventStateInfo findEventStates() throws SQLException {
        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            /*
                EXAMPLE DATA STRUCTURE USAGE
                ============================================
                EventStateInfo info = new EventStateInfo(50);
                info.addState("Kentucky");
                info.addState("Hawaii");
                info.addState("New Hampshire");
                return info;
            */
            ResultSet rst = stmt.executeQuery(
                "SELECT C.STATE_NAME, COUNT(DISTINCT EVENT_ID) AS nevent " +
                "FROM " + EventsTable + " E " +
                "JOIN "+ CitiesTable + " C " +
                "ON E.EVENT_CITY_ID = C.CITY_ID " +
                "GROUP BY C.STATE_NAME " +
                "ORDER BY COUNT(DISTINCT EVENT_ID) DESC, C.STATE_NAME ASC"
            );

            // collect all states using a List. Get the first row, and record the max events.
            // if the later rows have same amount of events as the first row, add the state to the list
            int maxCount = 0;
            int count = 0;
            
            while(rst.next()){

                // (B) Find the number of events held in the states identified in (A)

                if(rst.isFirst()){
                    maxCount = rst.getInt(2);
                }
                if(rst.getInt(2) == maxCount){
                    count += rst.getInt(2);
                }

                
            }

            EventStateInfo res = new EventStateInfo(count);
            Statement stmt2 = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly);
                    ResultSet rst2 = stmt2.executeQuery(
                        "SELECT C.STATE_NAME " +
                        "FROM " + EventsTable + " E " +
                        "JOIN "+ CitiesTable + " C " + 
                        "ON E.EVENT_CITY_ID = C.CITY_ID " +
                        "GROUP BY C.STATE_NAME " +
                        "HAVING COUNT(DISTINCT EVENT_ID) = " + maxCount

                        
                        );

                while(rst2.next()){
                    // (A) Find the name of the state or states in which the most events are held
                    res.addState(rst2.getString(1));
                   
                }

            // Close resources
            rst2.close();
            stmt2.close();
            rst.close();
            stmt.close();



            return res; // 
        } catch (SQLException e) {
            System.err.println(e.getMessage());
            return new EventStateInfo(-1);
        }
    }

    @Override
    // Query 8
    // -----------------------------------------------------------------------------------
    // GOALS: (A) Find the ID, first name, and last name of the oldest friend of the user
    //            with User ID <userID>
    //        (B) Find the ID, first name, and last name of the youngest friend of the user
    //            with User ID <userID>
     public AgeInfo findAgeInfo(long userID) throws SQLException {
        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            /*
                EXAMPLE DATA STRUCTURE USAGE
                ============================================
                UserInfo old = new UserInfo(12000000, "Galileo", "Galilei");
                UserInfo young = new UserInfo(80000000, "Neil", "deGrasse Tyson");
                return new AgeInfo(old, young);
            */

            //(A) Find the ID, first name, and last name of the oldest friend of the user
    //             with User ID <userID>
            ResultSet rst = stmt.executeQuery(
                "SELECT UIDF AS USER_ID, FIRST_NAME, LAST_NAME " +
                    "FROM (" +
                        "SELECT FF.USER_ID, FF.UIDF, UU.FIRST_NAME, UU.LAST_NAME, " +
                        "UU.YEAR_OF_BIRTH, UU.MONTH_OF_BIRTH, UU.DAY_OF_BIRTH " +
                        "FROM (" +
                            "SELECT U.USER_ID, " +
                                "CASE WHEN U.USER_ID = F.USER1_ID THEN F.USER2_ID " +
                                    "WHEN U.USER_ID = F.USER2_ID THEN F.USER1_ID " +
                                    "END AS UIDF " +
                            "FROM " + UsersTable + " U, " +
                                FriendsTable + " F " +
                            "WHERE (U.USER_ID = F.USER1_ID OR " +
                                "U.USER_ID = F.USER2_ID) AND " +
                                "U.USER_ID = "+ userID +") FF "+
                        "INNER JOIN " + UsersTable + " UU " +
                            "ON (FF.UIDF = UU.USER_ID) " +
                        "ORDER BY UU.YEAR_OF_BIRTH ASC, " +
                            "UU.MONTH_OF_BIRTH ASC, " +
                            "UU.DAY_OF_BIRTH ASC) " +
                    "WHERE ROWNUM = 1"
            );
            int oldID = -1;
            String oldFirst = " ";
            String oldLast = " ";
            while(rst.next()){
                if(rst.isFirst()){
                    oldID = rst.getInt(1);
                    oldFirst = rst.getString(2);
                    oldLast = rst.getString(3);
                }
            }
            UserInfo oldF = new UserInfo(oldID, oldFirst, oldLast);

             Statement stmt2 = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly);
                ResultSet rst2 = stmt2.executeQuery(
                    "SELECT UIDF AS USER_ID, FIRST_NAME, LAST_NAME " +
                "FROM ( " +
                    "SELECT FF.USER_ID, FF.UIDF, UU.FIRST_NAME, UU.LAST_NAME, " +
                    "UU.YEAR_OF_BIRTH, UU.MONTH_OF_BIRTH, UU.DAY_OF_BIRTH " +
                    "FROM ("+
                        "SELECT U.USER_ID, " +
                            "CASE WHEN U.USER_ID = F.USER1_ID THEN F.USER2_ID " +
                                "WHEN U.USER_ID = F.USER2_ID THEN F.USER1_ID " +
                                "END AS UIDF " +
                        "FROM " + UsersTable + " U, " +
                            FriendsTable +" F " +
                        "WHERE (U.USER_ID = F.USER1_ID OR " +
                            "U.USER_ID = F.USER2_ID) AND " +
                            "U.USER_ID = "+ userID +") FF " +
                    "INNER JOIN " + UsersTable + " UU " +
                        "ON (FF.UIDF = UU.USER_ID) " +
                    "ORDER BY UU.YEAR_OF_BIRTH DESC, " +
                        "UU.MONTH_OF_BIRTH DESC, " +
                        "UU.DAY_OF_BIRTH DESC) " +
                "WHERE ROWNUM = 1"
                    );
            int yId = -1;
            String yFirst = " ";
            String yLast = " ";
            while(rst2.next()){
                if(rst2.isFirst()){
                    yId = rst2.getInt(1);
                    yFirst = rst2.getString(2);
                    yLast = rst2.getString(3);
                }
            }
            UserInfo youngF = new UserInfo(yId, yFirst,yLast);
            AgeInfo res = new AgeInfo(oldF,youngF);

            // Close resources
            rst2.close();
            stmt2.close();
            rst.close();
            stmt.close();

            return res; // placeholder for compilation
        } catch (SQLException e) {
            System.err.println(e.getMessage());
            return new AgeInfo(new UserInfo(-1, "ERROR", "ERROR"), new UserInfo(-1, "ERROR", "ERROR"));
        }
    }

    @Override
    // Query 9
    // -----------------------------------------------------------------------------------
    // GOALS: (A) Find all pairs of users that meet each of the following criteria
    //              (i) same last name
    //              (ii) same hometown
    //              (iii) are friends
    //              (iv) less than 10 birth years apart
    public FakebookArrayList<SiblingInfo> findPotentialSiblings() throws SQLException {
        FakebookArrayList<SiblingInfo> results = new FakebookArrayList<SiblingInfo>("\n");

        try (Statement stmt = oracle.createStatement(FakebookOracleConstants.AllScroll,
                FakebookOracleConstants.ReadOnly)) {
            /*
                EXAMPLE DATA STRUCTURE USAGE
                ============================================
                UserInfo u1 = new UserInfo(81023, "Kim", "Kardashian");
                UserInfo u2 = new UserInfo(17231, "Kourtney", "Kardashian");
                SiblingInfo si = new SiblingInfo(u1, u2);
                results.add(si);
            */

        ResultSet rst = stmt.executeQuery(
            "SELECT DISTINCT U1.USER_ID AS UID1, U1.FIRST_NAME, U1.LAST_NAME, " +
             "U2.USER_ID AS UID2, U2.FIRST_NAME, U2.LAST_NAME " +
            "FROM "+ UsersTable +" U1, " +
                UsersTable + " U2, " +
                FriendsTable + " F, " +
                HometownCitiesTable + " H1, " +
                HometownCitiesTable + " H2, " +
                CitiesTable + " C " +
            "WHERE U1.LAST_NAME = U2.LAST_NAME " +
                "AND U1.USER_ID = H1.USER_ID AND U2.USER_ID = H2.USER_ID AND H1.HOMETOWN_CITY_ID = H2.HOMETOWN_CITY_ID "+
                "AND (U1.USER_ID = F.USER1_ID " +
                    "AND U2.USER_ID = F.USER2_ID " +
                    "OR U1.USER_ID = F.USER2_ID " +
                    "AND U2.USER_ID = F.USER1_ID) " +
                "AND ABS(U1.YEAR_OF_BIRTH - U2.YEAR_OF_BIRTH) < 10 " +
                "AND U1.USER_ID < U2.USER_ID " +
            "ORDER BY U1.USER_ID ASC, U2.USER_ID ASC"

        );

        while(rst.next()){
            UserInfo u1 = new UserInfo(rst.getInt(1), rst.getString(2),rst.getString(3));
            UserInfo u2 = new UserInfo(rst.getInt(4), rst.getString(5),rst.getString(6));
            SiblingInfo cur = new SiblingInfo(u1,u2);
            results.add(cur);

        }

         // * Close resources being used
        rst.close();
        stmt.close(); 


        } catch (SQLException e) {
            System.err.println(e.getMessage());
        }

        return results;
    }

    // Member Variables
    private Connection oracle;
    private final String UsersTable = FakebookOracleConstants.UsersTable;
    private final String CitiesTable = FakebookOracleConstants.CitiesTable;
    private final String FriendsTable = FakebookOracleConstants.FriendsTable;
    private final String CurrentCitiesTable = FakebookOracleConstants.CurrentCitiesTable;
    private final String HometownCitiesTable = FakebookOracleConstants.HometownCitiesTable;
    private final String ProgramsTable = FakebookOracleConstants.ProgramsTable;
    private final String EducationTable = FakebookOracleConstants.EducationTable;
    private final String EventsTable = FakebookOracleConstants.EventsTable;
    private final String AlbumsTable = FakebookOracleConstants.AlbumsTable;
    private final String PhotosTable = FakebookOracleConstants.PhotosTable;
    private final String TagsTable = FakebookOracleConstants.TagsTable;
}
