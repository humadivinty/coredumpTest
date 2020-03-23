# coredumpTest
 use Stackwalker to record  stack info when creash
注意：
1、g3log本身带有异常捕获功能，如果要取消，需要在编译的时候设置相关项
2、g3log的异常捕获和stackwalker的异常捕获不能共存，用了g3log的异常捕获后，stackwalker捕获到的异常就回不正确，所以两者不能同时使用
