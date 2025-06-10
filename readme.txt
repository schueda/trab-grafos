Para entradas do nome do grafo com espaço, o programa considera que o nome do grafo é apenas a primeira parte da string até o primeiro espaço.
Para entradas de vértices ou arestas fora da especificação, o programa aceita a primeira parte da string até o primeiro espaço como o nome de um vértice sozinho.
Para inserção de arestas mais eficiente, foi utilizado um ponteiro para o último item da lista, de forma que não é necessário percorrer toda a lista para inserir uma aresta.
Da linha 434 até a linha 551, é definida uma Min-Heap, que é utilizada como fila de prioridade para que o algoritmo de Dijkstra execute de maneira mais eficiente.
As funções "vertices_corte" e "arestas_corte", assim como "v_corte" e "a_corte" utilizam praticamente o mesmo código, diferindo na comparação final para montagem da string de resposta.
Adotando uma postura de programação defensiva, o código apresenta diversos casos de erro marcados pela função "perror", de forma a evitar execuções imprevisíveis em casos de erro e identificar causas mais facilmente.

---SEGUNDA ENTREGA--
Estudando para a prova, foi encontrado um bug ao inserir um grafo de uma prova antiga. Os vértices poderiam ser vértice de corte por conta de mais de um filho não ter low-point maior que seu nível, e isso faria com que o vértice fosse adicionado duas vezes a lista de vértices de corte. Para solucionar bastou colocar um "break", para sair do loop de verificação após encontrar o primeiro motivo para que o vértice fosse de corte.

--TERCEIRA ENTREGA--
Estudando para a prova, foi encontrado um bug na função "Dijkstra". Ao descobrir uma nova menor distância para um vértice, sua posição não era atualizada na fila de prioridade, fazendo com que o vértice pudesse não ser selecionado como vértice de menor distância em alguma iteração do algoritmo. Isso faria com que o algoritmo respondesse o valor errado de maior distância. Para corrigir, a struct "vertice" recebeu um novo campo "fila_pos", que armazena a posição na fila de um vértice. Então, ao atualizar o valor da distância de um vértice, é chamada a função sobe, para arrumar a posição do vértice na min-heap.