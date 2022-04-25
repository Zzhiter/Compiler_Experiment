class Lexer:
    def __init__(self, file_name):
        self.file_name = file_name
        # 关键字对应的类别码
        self.code_type = {  # 关键字
            'int': 0,
            'double': 1,
            'if': 2,
            'then': 3,
            'else': 4,
            'while': 5,
            'do': 6,
            'true': 7,
            'false': 8,
            'return': 9,
            'for': 10,
            'bool': 11,
            'string': 12,
            'include': 13,

            # id
            'id': 14,

            # 数字类型
            'integer': 15,
            'decimal': 16,
            'String': 17,

            # 各种符号
            '=': 18,
            '==': 19,
            '>': 20,
            '>=': 21,
            '<': 22,
            '<=': 23,
            '!=': 24,
            '+': 25,
            '-': 26,
            '*': 27,
            '/': 28,
            '&&': 29,
            '||': 30,
            '{': 31,
            '}': 32,
            '(': 33,
            ')': 34,
            ';': 35,
            ',': 36,
            '[': 37,
            ']': 38,
            '.': 39,
            '#': 40,
            "'": 41,
            '"': 42, }
        self.code = ''
        self.code_length = 0
        self.token = []
        self.read_file()
        self.pre_process()

    # 读取要处理的代码文件
    def read_file(self):
        assert len(self.file_name) > 0, "The input file name error"

        with open(self.file_name, 'r') as f:
            content = f.readlines()

        for i in content:
            self.code += str(i)


    # 删除注释内容
    def pre_process(self):
        # 删除//以后的 整行的内容
        left = 0
        while self.code.find('//', left) != -1:
            left = self.code.find('//', left)
            print(self.code)
            right = self.code.find('\n', left)
            self.code = self.code[:left] + self.code[right + 1:]

        # 删除/* */内的所有内容
        left = 0
        while self.code.find('/*', left) != -1:
            left = self.code.find('/*', left)
            right = self.code.find('*/', left)
            self.code = self.code[:left] + self.code[right + 1:]

        self.code_length = len(self.code)
        print("preprocess successfully!")
        print('处理后的代码为：' + self.code)


if __name__ == '__main__':
    lexer = Lexer('test_code.c')
