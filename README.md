# Árvore rubro-negra com persistência parcial
    
## Uso
Antes de executar o programa, é necessário criar dois arquivos: um para entrada e outro para saída. O arquivo de entrada conterá os comandos a serem executados, enquanto o de saída conterá os resultados.

## Exemplo de uso (Linux com g++)
1. Crie os arquivos
    ```
    touch input.txt
    touch outpt.txt
    ```
2. Compile o programa
    ```
    g++ RBTreeFileHandler.cpp -o RBTreeFileHandler
    ```
3. Execute o programa
    ```
    ./RBTreeFileHandler input.txt output.txt
    ```
### Ou para interagir com a árvore pela linha de comando
```
g++ ViewTree.cpp -o ViewTree
./ViewTree
```
