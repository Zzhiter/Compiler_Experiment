class Lexer:
    def __init__(self, file_name):
        self.file_name = file_name
        # 关键字对应的类别码
        self.code_type = {
            # 关键字
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

            # identifier 标识符
            'id': 14,

            # 常数：包含整形，浮点型，字符型
            'integer': 15,
            'decimal': 16,
            'String': 17,

            # 界限符
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
        # 双目运算符映射
        self.op_map = {
            '>': '=',
            '<': '=',
            '&': '&',
            '|': '|',
            '=': '=',
            '!': '='
        }
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

    # 讲word添加到token表中
    def insert_to_token(self, word: str, type):
        """
        param word: 当前读取的单词
        param type: 该单词的属性
        return:None
        """
        keys = self.code_type.keys()
        if word in keys:
            self.token.append({'word': word, 'type': self.code_type[word]})
        else:
            self.token.append({'word': word, 'type': self.code_type[type]})

    # 处理双目运算符
    def is_bincolar_operator(self, i, word, character):
        if self.code[i + 1] == self.op_map[character]:
            i += 1
            word += self.code[i]
        self.insert_to_token(word, word)

        return i

    # 扫描程序
    def scan(self):
        i = 0
        while i < self.code_length:
            character = self.code[i]
            word = ''

            # 过滤掉回车和空格
            while character == '\n' or character == ' ':
                i += 1
                if i >= self.code_length:
                    return
                character = self.code[i]

            word += character

            # 判断当前word是否是运算符
            if character in ['&', '|', '!', '>', '<', '=']:
                print("character %s in " % character)
                i = self.is_bincolar_operator(i, word, character)
                i += 1

            # 判断是否是常量数字
            elif character.isdigit():
                while character.isdigit():
                    i += 1
                    character = self.code[i]
                    word += character
                word = word[:-1]
                if character != '.':
                    self.insert_to_token(word, 'integer')
                else:
                    word += character
                    i += 1
                    character = self.code[i]
                    while character.isdigit():
                        word += character
                        i += 1
                        character = self.code[i]
                    self.insert_to_token(word, 'decimal')

            # 判断是否为字符串常量
            elif character == "'":
                self.insert_to_token(word, word)
                s = ''
                i += 1
                while self.code[i] != "'":
                    s += self.code[i]
                    i += 1
                self.insert_to_token(s, 'String')
                self.insert_to_token("'", "'")
                i += 1

            # 判断是否为标识符
            elif character.isalpha():
                i += 1
                while self.code[i].isalpha() or self.code.isdigit():
                    word += self.code[i]
                    i += 1
                self.insert_to_token(word, 'id')
            # 界限符
            else:
                self.insert_to_token(word, word)
                i += 1

    # 输出到控制台并保存到文件
    def save(self):
        for _, token in enumerate(self.token):
            print('<%s, %s>' % (token['word'], token['type']))

        with open('Symbol_table.txt', 'w') as f:
            for i in self.code_type:
                f.write('<' + str(i) + ', ' + str(self.code_type[i]) + \
                        '>\n')

        with open('token_list.txt', 'w') as f:
            for _, token in enumerate(self.token):
                f.write('<%s, %s>\n' % (token['word'], token['type']))


if __name__ == '__main__':
    lexer = Lexer('test_code.c')
    lexer.scan()
    lexer.save()